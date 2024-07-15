// Engine Includes.
#include "VertexBuffer.hpp"

namespace Engine
{
	VertexBuffer::VertexBuffer()
		:
		id( -1 ),
		vertex_count( 0 ),
		size( 0 )
	{
	}

	VertexBuffer::VertexBuffer( const void* vertex_data, const unsigned int vertex_count, const unsigned int size, const GLenum usage )
		:
		id( -1 ),
		vertex_count( vertex_count ),
		size( size )
	{
		ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to VertexBuffer::VertexBuffer( const void* vertex_data, const unsigned int size, const GLenum usage ) is zero!" );

		GLCALL( glGenBuffers( 1, &id ) );
		Bind();
		GLCALL( glBufferData( GL_ARRAY_BUFFER, size, ( void* )vertex_data, usage ) );
	}

	VertexBuffer::VertexBuffer( VertexBuffer&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		vertex_count( std::exchange( donor.vertex_count, 0 ) ),
		size( std::exchange( donor.size, 0 ) )
	{
	}

	VertexBuffer& VertexBuffer::operator = ( VertexBuffer&& donor )
	{
		id           = std::exchange( donor.id,				-1 );
		vertex_count = std::exchange( donor.vertex_count,	 0 );
		size         = std::exchange( donor.size,			 0 );

		return *this;
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
		ASSERT_DEBUG_ONLY( IsValid() && "Attempting Bind() on VertexBuffer with zero size!" );

		GLCALL( glBindBuffer( GL_ARRAY_BUFFER, id ) );
	}
}
