// Engine Includes.
#include "Graphics.h"
#include "VertexArray.h"

// std Includes.
#include <utility>

namespace Engine
{
	VertexArray::VertexArray()
		:
		id( -1 ),
		vertex_buffer_id( -1 ),
		index_buffer_id( -1 ),
		vertex_count( 0 ),
		index_count( 0 )
	{
	}

	VertexArray::VertexArray( VertexArray&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		vertex_buffer_id( std::exchange( donor.vertex_buffer_id, -1 ) ),
		index_buffer_id( std::exchange( donor.index_buffer_id, -1 ) ),
		vertex_count( std::exchange( donor.vertex_count, 0 ) ),
		index_count( std::exchange( donor.index_count, 0 ) )
	{
	}

	VertexArray& VertexArray::operator=( VertexArray&& donor )
	{
		id               = std::exchange( donor.id,					-1 );
		vertex_buffer_id = std::exchange( donor.vertex_buffer_id,	-1 );
		index_buffer_id  = std::exchange( donor.index_buffer_id,	-1 );
		vertex_count     = std::exchange( donor.vertex_count,		 0 );
		index_count      = std::exchange( donor.index_count,		 0 );

		return *this;
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout )
		:
		id( -1 ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( -1 ),
		vertex_count( vertex_buffer.VertexCount() ),
		index_count( 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout, const IndexBuffer& index_buffer )
		:
		id( -1 ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( index_buffer.Id() ),
		vertex_count( vertex_buffer.VertexCount() ),
		index_count( index_buffer.IndexCount() )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );
		index_buffer.Bind();

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout, const std::optional< IndexBuffer >& index_buffer )
		:
		id( -1 ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( index_buffer ? index_buffer->Id() : -1 ),
		vertex_count( vertex_buffer.VertexCount() ),
		index_count( index_buffer ? index_buffer->IndexCount() : 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );
		if( index_buffer )
			index_buffer->Bind();

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::~VertexArray()
	{
		if( IsValid() )
		{
			GLCALL( glDeleteVertexArrays( 1, &id ) );
		}
	}

	void VertexArray::Bind() const
	{
		GLCALL( glBindVertexArray( id ) );
	}

	void VertexArray::Unbind() const
	{
		GLCALL( glBindVertexArray( 0 ) );
	}

	void VertexArray::CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout )
	{
		GLCALL( glGenVertexArrays( 1, &id ) );
		Bind();

		vertex_buffer.Bind();
		vertex_buffer_layout.SetAndEnableAttributes();
	}
}