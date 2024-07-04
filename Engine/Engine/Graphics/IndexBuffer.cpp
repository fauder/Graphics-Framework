// Engine Includes.
#include "IndexBuffer.h"

namespace Engine
{
	IndexBuffer::IndexBuffer()
		:
		id( -1 ),
		size( 0 )
	{
	}

	IndexBuffer::IndexBuffer( IndexBuffer&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		size( std::exchange( donor.size, 0 ) )
	{
	}

	IndexBuffer& IndexBuffer::operator = ( IndexBuffer&& donor )
	{
		id   = std::exchange( donor.id, -1 );
		size = std::exchange( donor.size, 0 );

		return *this;
	}

	IndexBuffer::IndexBuffer( const unsigned int* index_data, const unsigned int size, const GLenum usage )
		:
		id( -1 ),
		size( size )
	{
		ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to IndexBuffer::IndexBuffer( const unsigned int* index_data, const unsigned int size, const GLenum usage ) is zero!" );

		GLCALL( glGenBuffers( 1, &id ) );
		Bind();
		GLCALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, ( void* )index_data, usage ) );
	}

	IndexBuffer::~IndexBuffer()
	{
		if( IsValid() )
		{
			GLCALL( glDeleteBuffers( 1, &id ) );
		}
	}

	void IndexBuffer::Bind() const
	{
		ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to IndexBuffer::Bind() is zero!" );
		
		GLCALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id ) );
	}
}
