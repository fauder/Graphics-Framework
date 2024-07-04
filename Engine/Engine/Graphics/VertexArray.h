#pragma once

// Engine Includes.
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"

namespace Engine
{
	class VertexArray
	{
	public:
		VertexArray();
		VertexArray( const VertexArray& right_hand_side )				= delete;
		VertexArray& operator =( const VertexArray& right_hand_side )	= delete;
		VertexArray( VertexArray&& donor );
		VertexArray& operator =( VertexArray&& donor );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout, const IndexBuffer& index_buffer );
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		unsigned int VertexCount() const;
		unsigned int IndexCount() const;

	private:
		void CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout );

	private:
		unsigned int id;

		unsigned int vertex_buffer_id;
		unsigned int index_buffer_id;

		unsigned int vertex_count;
		unsigned int index_count;

		bool is_valid;
	};
}
