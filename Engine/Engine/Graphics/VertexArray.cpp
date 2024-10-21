// Engine Includes.
#include "Graphics.h"
#include "VertexArray.h"

// std Includes.
#include <utility>

namespace Engine
{
	VertexArray::VertexArray( const std::string& name )
		:
		id( {} ),
		name( name ),
		vertex_buffer_id( {} ),
		index_buffer_id( {} ),
		vertex_count( 0 ),
		index_count( 0 )
	{
	}

	VertexArray::VertexArray( VertexArray&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		name( std::exchange( donor.name, {} ) ),
		vertex_buffer_id( std::exchange( donor.vertex_buffer_id, {} ) ),
		index_buffer_id( std::exchange( donor.index_buffer_id, {} ) ),
		vertex_count( std::exchange( donor.vertex_count, 0 ) ),
		index_count( std::exchange( donor.index_count, 0 ) )
	{
	}

	VertexArray& VertexArray::operator=( VertexArray&& donor )
	{
		id               = std::exchange( donor.id,					{} );
		name             = std::exchange( donor.name,				{} );
		vertex_buffer_id = std::exchange( donor.vertex_buffer_id,	{} );
		index_buffer_id  = std::exchange( donor.index_buffer_id,	{} );
		vertex_count     = std::exchange( donor.vertex_count,		 0 );
		index_count      = std::exchange( donor.index_count,		 0 );

		return *this;
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_buffer_layout, const std::string& name )
		:
		id( {} ),
		name( name ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( {} ),
		vertex_count( vertex_buffer.Count() ),
		index_count( 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_buffer_layout,
							  const std::optional< IndexBuffer >& index_buffer,
							  const std::string& name )
		:
		id( {} ),
		name( name ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( index_buffer
							? index_buffer->Id()
							: IndexBuffer::ID() ),
		vertex_count( vertex_buffer.Count() ),
		index_count( index_buffer
						? index_buffer->Count()
						: 0 )
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
			glDeleteVertexArrays( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray( id.Get() );
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray( 0 );
	}

	void VertexArray::CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_buffer_layout )
	{
		glGenVertexArrays( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_VERTEX_ARRAY, id.Get(), name );
#endif // _DEBUG

		vertex_buffer.Bind();
		vertex_buffer_layout.SetAndEnableAttributes();
	}
}