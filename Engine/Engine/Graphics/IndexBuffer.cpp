// Engine Includes.
#include "IndexBuffer.h"

namespace Engine
{
	IndexBuffer::IndexBuffer( const unsigned int* index_data, const unsigned int size, const GLenum usage )
		:
		id( -1 ),
		size( size )
	{
		GLCALL( glGenBuffers( 1, &id ) );
		Bind();
		GLCALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, ( void* )index_data, usage ) );
	}

	IndexBuffer::~IndexBuffer()
	{
		GLCALL( glDeleteBuffers( 1, &id ) );
	}

	void IndexBuffer::Bind() const
	{
		GLCALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, id ) );
	}

	unsigned int IndexBuffer::Size() const
	{
		return size;
	}
}
