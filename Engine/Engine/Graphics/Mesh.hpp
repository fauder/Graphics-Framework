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
		enum class PrimitiveType
		{
			Points			= GL_POINTS,
			Lines			= GL_LINES,
			Line_loop		= GL_LINE_LOOP,
			Line_strip		= GL_LINE_STRIP,
			Triangles		= GL_TRIANGLES,
			Triangle_strip	= GL_TRIANGLE_STRIP,
			Triangle_fan	= GL_TRIANGLE_FAN,
			Quads			= GL_QUADS
		};

	public:
		Mesh()
			:
			primitive_type( PrimitiveType::Triangles )
		{}

		Mesh( const Mesh& other )				= default;
		Mesh& operator = ( const Mesh& other )	= default;
		Mesh( Mesh&& donor )					= default;
		Mesh& operator = ( Mesh&& donor )		= default;

		Mesh( std::vector< Vector3		>&& positions, 
			  std::vector< Vector3		>&& normals        = {},
			  std::vector< Vector2		>&& uvs_0          = {}, 
			  std::vector< unsigned int >&& indices        = {},
			  const PrimitiveType			primitive_type = PrimitiveType::Triangles,
			  const GLenum usage                           = GL_STATIC_DRAW,
			  /* Below are seldom used so they come after. */
			  std::vector< Vector2		>&& uvs_1          = {},
			  std::vector< Vector2		>&& uvs_2          = {}, 
			  std::vector< Vector2		>&& uvs_3          = {},
			  std::vector< Color3		>&& colors_rgb     = {}, 
			  std::vector< Color4		>&& colors_rgba    = {} )
			:
			primitive_type( primitive_type ),
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

			vertex_buffer        = VertexBuffer( std::span( interleaved_vertices ), usage );
			vertex_buffer_layout = VertexBufferLayout( AttributeCountsAndTypes( positions, normals, uvs_0, uvs_1, uvs_2, uvs_3, colors_rgb, colors_rgba ) );
			index_buffer         = indices.empty() ? std::nullopt : std::optional< IndexBuffer >( std::in_place, indices, usage );
			vertex_array         = VertexArray( vertex_buffer, vertex_buffer_layout, index_buffer );
		}

		inline ~Mesh()
		{}

	/* Queries: */
		inline PrimitiveType Primitive() const { return primitive_type; }

		inline int VertexCount() const { return vertex_buffer.Count(); }
		inline int IndexCount()  const { return index_buffer.has_value() ? index_buffer->Count() : 0; }

		inline bool IsIndexed()  const { return IndexCount(); }

	/* Index Data: */
		inline const std::vector< unsigned int >& Indices()	const { return indices; };
		inline const float* Indices_Raw() const { return reinterpret_cast< const float* >( indices.data() ); };

	/* Vertex Data: */
		inline const std::vector< Vector3 >& Positions()	const { return positions;	};
		inline const std::vector< Vector3 >& Normals()		const { return normals;		};
		inline const std::vector< Vector2 >& Uvs_0()		const { return uvs_0;		};
		inline const std::vector< Vector2 >& Uvs_1()		const { return uvs_1;		};
		inline const std::vector< Vector2 >& Uvs_2()		const { return uvs_2;		};
		inline const std::vector< Vector2 >& Uvs_3()		const { return uvs_3;		};
		inline const std::vector< Color3  >& Colors_rgb()	const { return colors_rgb;	};
		inline const std::vector< Color4  >& Colors_rgba()	const { return colors_rgba; };

		inline const float* Positions_Raw()		const { return reinterpret_cast< const float* >( positions.data()	); };
		inline const float* Normals_Raw()		const { return reinterpret_cast< const float* >( normals.data()		); };
		inline const float* Uvs_0_Raw()			const { return reinterpret_cast< const float* >( uvs_0.data()		); };
		inline const float* Uvs_1_Raw()			const { return reinterpret_cast< const float* >( uvs_1.data()		); };
		inline const float* Uvs_2_Raw()			const { return reinterpret_cast< const float* >( uvs_2.data()		); };
		inline const float* Uvs_3_Raw()			const { return reinterpret_cast< const float* >( uvs_3.data()		); };
		inline const float* Colors_rgb_Raw()	const { return reinterpret_cast< const float* >( colors_rgb.data()	); };
		inline const float* Colors_rgba_Raw()	const { return reinterpret_cast< const float* >( colors_rgba.data()	); };

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
		PrimitiveType primitive_type;

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