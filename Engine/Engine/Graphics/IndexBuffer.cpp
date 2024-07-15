// Engine Includes.
#include "IndexBuffer.hpp"

namespace Engine
{
	IndexBuffer::IndexBuffer()
		:
		id( -1 ),
		index_count( 0 ),
		size( 0 )
	{
	}

	IndexBuffer::IndexBuffer( const unsigned int* index_data, const unsigned int count, const GLenum usage )
		:
		id( -1 ),
		index_count( count ),
		size( count * sizeof( unsigned int ) )
	{
		ASSERT_DEBUG_ONLY( IsValid() && "'count' parameter passed to IndexBuffer::IndexBuffer( const unsigned int* index_data, const unsigned int count, const GLenum usage ) is zero!" );

		GLCALL( glGenBuffers( 1, &id ) );
		Bind();
		GLCALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, ( void* )index_data, usage ) );
	}

	IndexBuffer::IndexBuffer( IndexBuffer&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		index_count( std::exchange( donor.index_count, 0 ) ),
		size( std::exchange( donor.size, 0 ) )
	{
	}

	IndexBuffer& IndexBuffer::operator = ( IndexBuffer&& donor )
	{
		id          = std::exchange( donor.id,			-1 );
		index_count = std::exchange( donor.index_count,  0 );
		size        = std::exchange( donor.size,		 0 );

		return *this;
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
		ASSERT_DEBUG_ONLY( IsValid() && "Attempting Bind() on IndexBuffer with zero size!" );
		
		GLCALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id ) );
	}
}
