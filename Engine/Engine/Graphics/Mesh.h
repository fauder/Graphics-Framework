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
		Mesh();

		/* Prevent copying but allow moving: */
		Mesh( const Mesh& other )				= delete;
		Mesh& operator = ( const Mesh& other )	= delete;
		Mesh( Mesh&& donor )					= default;
		Mesh& operator = ( Mesh&& donor )		= default;

		Mesh( std::vector< Vector3			>&& positions, 
			  const std::string&				name			= {},
			  std::vector< Vector3			>&& normals			= {},
			  std::vector< Vector2			>&& uvs_0			= {}, 
			  std::vector< std::uint32_t	>&& indices		    = {},
			  const PrimitiveType				primitive_type	= PrimitiveType::Triangles,
			  const GLenum						usage			= GL_STATIC_DRAW,
			  /* Below are seldom used so they come after. */
			  std::vector< Vector2			>&& uvs_1			= {},
			  std::vector< Vector2			>&& uvs_2			= {}, 
			  std::vector< Vector2			>&& uvs_3			= {},
			  std::vector< Color4			>&& colors_rgba		= {} );

		Mesh( const Mesh& other,
			  const std::initializer_list< VertexInstanceAttribute > instanced_attributes,
			  const std::vector< float >& instance_data,
			  const int instance_count,
			  const GLenum usage = GL_STATIC_DRAW );

		~Mesh();

	/*
	 * Usage:
	 */

		inline void Bind() const { vertex_array.Bind(); }
		void Update( const void* data ) const;
		void Update_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const;
		void UpdateInstanceData( const void* data ) const;
		void UpdateInstanceData_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const;

	/*
	 * Queries:
	 */

		inline const std::string& Name() const { return name; }

		inline PrimitiveType Primitive() const { return primitive_type; }

		inline int VertexCount() const { return vertex_buffer.Count(); }
		inline int IndexCount()  const { return index_buffer.has_value() ? index_buffer->Count() : 0; }

		inline bool HasIndices() const { return IndexCount(); }

		inline bool HasInstancing() const { return ( bool )instance_buffer; }
		inline int InstanceCount() const { return instance_count; }

		inline bool IsCompatibleWith( const VertexLayout& other_vertex_layout ) const { return vertex_layout.IsCompatibleWith( other_vertex_layout ); }

	/*
	 * Index Data:
	 */

		inline const std::vector< std::uint32_t >&	Indices()		const { return indices; };
		inline const std::uint32_t*					Indices_Raw()	const { return indices.data(); };
		inline constexpr GLenum						IndexType()		const { return GL_UNSIGNED_INT; }

	/*
	 * Vertex Data:
	 */

		inline const std::vector< Vector3 >& Positions()	const { return positions;	};
		inline const std::vector< Vector3 >& Normals()		const { return normals;		};
		inline const std::vector< Vector2 >& Uvs_0()		const { return uvs_0;		};
		inline const std::vector< Vector2 >& Uvs_1()		const { return uvs_1;		};
		inline const std::vector< Vector2 >& Uvs_2()		const { return uvs_2;		};
		inline const std::vector< Vector2 >& Uvs_3()		const { return uvs_3;		};
		inline const std::vector< Color4  >& Colors_rgba()	const { return colors_rgba; };

		inline const float* Positions_Raw()		const { return reinterpret_cast< const float* >( positions.data()	); };
		inline const float* Normals_Raw()		const { return reinterpret_cast< const float* >( normals.data()		); };
		inline const float* Uvs_0_Raw()			const { return reinterpret_cast< const float* >( uvs_0.data()		); };
		inline const float* Uvs_1_Raw()			const { return reinterpret_cast< const float* >( uvs_1.data()		); };
		inline const float* Uvs_2_Raw()			const { return reinterpret_cast< const float* >( uvs_2.data()		); };
		inline const float* Uvs_3_Raw()			const { return reinterpret_cast< const float* >( uvs_3.data()		); };
		inline const float* Colors_RGBA_Raw()	const { return reinterpret_cast< const float* >( colors_rgba.data()	); };

	private:
		static std::array< VertexAttribute, 7 > GatherAttributes( const std::vector< Vector3 >& positions, const std::vector< Vector3 >& normals,
																  const std::vector< Vector2 >& uvs_0, const std::vector< Vector2 >& uvs_1,
																  const std::vector< Vector2 >& uvs_2, const std::vector< Vector2 >& uvs_3,
																  const std::vector< Color4 >& colors_rgba );

 	private:
		std::string name;

		PrimitiveType primitive_type;

		std::vector< std::uint32_t > indices;

		std::vector< Vector3 > positions;
		std::vector< Vector3 > normals;
		std::vector< Vector2 > uvs_0;
		std::vector< Vector2 > uvs_1;
		std::vector< Vector2 > uvs_2;
		std::vector< Vector2 > uvs_3;
		std::vector< Color4  > colors_rgba;

		int instance_count;

		VertexBuffer vertex_buffer;
		VertexLayout vertex_layout;
		std::optional< IndexBuffer > index_buffer;
		std::optional< InstanceBuffer > instance_buffer;
		VertexArray vertex_array;
	};
}