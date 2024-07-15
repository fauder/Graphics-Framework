#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	class VertexBuffer
	{
	public:
		using ID = unsigned int;

	public:
		VertexBuffer();
		VertexBuffer( const void* vertex_data, const unsigned int vertex_count, const unsigned int size, const GLenum usage = GL_STATIC_DRAW );
		template< typename Collection >
		VertexBuffer( Collection&& vertex_data, const unsigned int vertex_count, const GLenum usage = GL_STATIC_DRAW )
			:
			id( -1 ),
			vertex_count( vertex_count ),
			size( ( unsigned int )vertex_data.size() * sizeof( std::remove_reference_t< decltype( vertex_data ) >::value_type ) )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'vertex_data' parameter passed to VertexBuffer::VertexBuffer< Collection&& >( const Collection&& vertex_data, const GLenum usage ) is empty!" );

			GLCALL( glGenBuffers( 1, &id ) );
			Bind();
			GLCALL( glBufferData( GL_ARRAY_BUFFER, size, ( void* )vertex_data.data(), usage ) );
		}

		/* Prohibit copying; It does not make sense two have multiple vertex buffers with the same Id. */
		VertexBuffer( const VertexBuffer& other )			   = delete;
		VertexBuffer& operator = ( const VertexBuffer& other ) = delete;

		/* Allow moving. */
		VertexBuffer( VertexBuffer&& donor );
		VertexBuffer& operator = ( VertexBuffer&& donor );
		
		~VertexBuffer();
		
		void Bind() const;
		
		inline ID			Id()			const { return id;				}
		inline unsigned int VertexCount()	const { return vertex_count;	}
		inline unsigned int Size()			const { return size;			}
		inline bool			IsValid()		const { return vertex_count;	} // Use the vertex count to implicitly define validness state.

	private:
		ID id;
		unsigned int vertex_count;
		unsigned int size;
	};
}
