#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	class IndexBuffer
	{
	public:
		IndexBuffer();
		IndexBuffer( const unsigned int* index_data, const unsigned int count, const GLenum usage );
		template< typename Collection >
		IndexBuffer( Collection&& index_data, const GLenum usage )
			:
			id( -1 ),
			index_count( ( int )index_data.size() ),
			size( index_count * sizeof( index_data.front() ) )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'index_data' parameter passed to IndexBuffer::IndexBuffer< Collection&& >( const Collection&& index_data, const GLenum usage ) is empty!" );

			GLCALL( glGenBuffers( 1, &id ) );
			Bind();
			GLCALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, ( void* )index_data.data(), usage ) );
		}

		/* Prohibit copying; It does not make sense two have multiple index buffers with the same Id. */
		IndexBuffer( const IndexBuffer& other )				 = delete;
		IndexBuffer& operator = ( const IndexBuffer& other ) = delete;

		/* Allow moving. */
		IndexBuffer( IndexBuffer&& donor );
		IndexBuffer& operator = ( IndexBuffer&& donor );

		~IndexBuffer();

		void Bind() const;

		inline unsigned int Id()			const { return id;			}
		inline unsigned int IndexCount()	const { return index_count;	}
		inline unsigned int Size()			const { return size;		}
		inline bool			IsValid()		const { return index_count; } // Use the index count to implicitly define validness state.

	private:
		unsigned int id;
		unsigned int index_count;
		unsigned int size;
	};
}
