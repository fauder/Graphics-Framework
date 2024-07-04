// Engine Includes.
#include "VertexBuffer.h"

namespace Engine
{
	VertexBuffer::VertexBuffer()
		:
		id( -1 ),
		size( 0 )
	{
	}

	VertexBuffer::VertexBuffer( VertexBuffer&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		size( std::exchange( donor.size, 0 ) )
	{
	}

	VertexBuffer& VertexBuffer::operator = ( VertexBuffer&& donor )
	{
		id   = std::exchange( donor.id, -1 );
		size = std::exchange( donor.size, 0 );

		return *this;
	}

	VertexBuffer::VertexBuffer( const void* vertex_data, const unsigned int size, const GLenum usage )
		:
		id( -1 ),
		size( size )
	{
		ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to VertexBuffer::VertexBuffer( const void* vertex_data, const unsigned int size, const GLenum usage ) is zero!" );

		GLCALL( glGenBuffers( 1, &id ) );
		Bind();
		GLCALL( glBufferData( GL_ARRAY_BUFFER, size, ( void* )vertex_data, usage ) );
	}

	VertexBuffer::~VertexBuffer()
	{
		if( IsValid() )
		{
			GLCALL( glDeleteBuffers( 1, &id ) );
		}
	}

	void VertexBuffer::Bind() const
	{
		ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to VertexBuffer::Bind() is zero!" );

		GLCALL( glBindBuffer( GL_ARRAY_BUFFER, id ) );
	}
}
