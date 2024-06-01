// Engine Includes.
#include "VertexBuffer.h"

namespace Engine
{
	VertexBuffer::VertexBuffer( const void* vertex_data, const unsigned int size, const GLenum usage )
		:
		id( -1 ),
		size( size )
	{
		GLCALL( glGenBuffers( 1, &id ) );
		Bind();
		GLCALL( glBufferData( GL_ARRAY_BUFFER, size, ( void* )vertex_data, usage ) );
	}

	VertexBuffer::~VertexBuffer()
	{
		GLCALL( glDeleteBuffers( 1, &id ) );
	}

	void VertexBuffer::Bind() const
	{
		GLCALL( glBindBuffer( GL_ARRAY_BUFFER, id ) );
	}

	unsigned int VertexBuffer::Size() const
	{
		return size;
	}
}
