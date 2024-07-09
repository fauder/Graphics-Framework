#pragma once

// Engine Includes.
#include "Color.hpp"
#include "MeshUtility.hpp"
#include "VertexArray.h"

// std Includes.
#include <array>

namespace Engine
{
	class Mesh
	{
	public:
		inline Mesh()
		{}

		Mesh( const Mesh& other )				= default;
		Mesh& operator = ( const Mesh& other )	= default;
		Mesh( Mesh&& donor )					= default;
		Mesh& operator = ( Mesh&& donor )		= default;

		Mesh( std::vector< Vector3 >&& positions, 
			  const GLenum usage                        = GL_STATIC_DRAW,
			  std::vector< unsigned int >&& indices     = {},
			  std::vector< Vector3		>&& normals     = {},
			  std::vector< Vector2		>&& uvs_0       = {}, 
			  std::vector< Vector2		>&& uvs_1       = {}, 
			  std::vector< Vector2		>&& uvs_2       = {}, 
			  std::vector< Vector2		>&& uvs_3       = {},
			  std::vector< Color3		>&& colors_rgb  = {}, 
			  std::vector< Color4		>&& colors_rgba = {} )
			:
			indices( indices ),
			positions( positions ),
			normals( normals ),
			uvs_0( uvs_0 ),
			uvs_1( uvs_1 ),
			uvs_2( uvs_2 ),
			uvs_3( uvs_3 ),
			colors_rgb( colors_rgb ),
			colors_rgba( colors_rgba )
		{
			unsigned int vertex_count_interleaved;
			const auto interleaved_vertices = MeshUtility::Interleave( vertex_count_interleaved, positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgb, colors_rgba );

			vertex_buffer        = VertexBuffer( interleaved_vertices, vertex_count_interleaved, usage );
			vertex_buffer_layout = VertexBufferLayout( AttributeCountsAndTypes( positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgb, colors_rgba ) );
			index_buffer         = indices.empty() ? std::nullopt : std::optional< IndexBuffer >( std::in_place, indices, usage );
			vertex_array         = VertexArray( vertex_buffer, vertex_buffer_layout, index_buffer );
		}

		inline ~Mesh()
		{}

		inline int VertexCount() const { return vertex_buffer.VertexCount(); }
		inline int IndexCount()  const { return index_buffer.has_value() ? index_buffer->Size() : 0; }

		inline void Bind() const { vertex_array.Bind(); }

	private:
		static std::array< VertexAttributeCountAndType, 8 > AttributeCountsAndTypes( const std::vector< Vector3 >& positions, const std::vector< Vector3 >& normals,
																					 const std::vector< Vector2 >& uvs_0, const std::vector< Vector2 >& uvs_1,
																					 const std::vector< Vector2 >& uvs_2, const std::vector< Vector2 >& uvs_3,
																					 const std::vector< Color3 >& colors_rgb, const std::vector< Color4 >& colors_rgba )
		{
			return
			{
				positions.empty()	? 0 : int( sizeof( positions.front() )	 / sizeof( float ) ),		GL_FLOAT,
				normals.empty()		? 0 : int( sizeof( normals.front() )	 / sizeof( float ) ),		GL_FLOAT,
				uvs_0.empty()		? 0 : int( sizeof( uvs_0.front() )		 / sizeof( float ) ),		GL_FLOAT,
				uvs_1.empty()		? 0 : int( sizeof( uvs_1.front() )		 / sizeof( float ) ),		GL_FLOAT,
				uvs_2.empty()		? 0 : int( sizeof( uvs_2.front() )		 / sizeof( float ) ),		GL_FLOAT,
				uvs_3.empty()		? 0 : int( sizeof( uvs_3.front() )		 / sizeof( float ) ),		GL_FLOAT,
				colors_rgb.empty()	? 0 : int( sizeof( colors_rgb.front() )	 / sizeof( float ) ),		GL_FLOAT,
				colors_rgba.empty() ? 0 : int( sizeof( colors_rgba.front() ) / sizeof( float ) ),		GL_FLOAT
			};
		}

 	private:
		std::vector< unsigned int > indices;

		std::vector< Vector3 > positions;
		std::vector< Vector3 > normals;
		std::vector< Vector2 > uvs_0;
		std::vector< Vector2 > uvs_1;
		std::vector< Vector2 > uvs_2;
		std::vector< Vector2 > uvs_3;
		std::vector< Color3  > colors_rgb;
		std::vector< Color4  > colors_rgba;

		VertexBuffer vertex_buffer;
		VertexBufferLayout vertex_buffer_layout;
		std::optional< IndexBuffer > index_buffer;
		VertexArray vertex_array;
	};

}