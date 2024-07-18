#pragma once

// Engine Includes.
#include "VertexBuffer.h"
#include "VertexBufferLayout.hpp"
#include "IndexBuffer.h"

// std Includes.
#include <optional>

namespace Engine
{
	class VertexArray
	{
	public:
		using ID = unsigned int;

	public:
		VertexArray();
		VertexArray( const VertexArray& right_hand_side )				= delete;
		VertexArray& operator =( const VertexArray& right_hand_side )	= delete;
		VertexArray( VertexArray&& donor );
		VertexArray& operator =( VertexArray&& donor );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout, const IndexBuffer& index_buffer );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout, const std::optional< IndexBuffer >& index_buffer );
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		inline ID			Id()			const { return id;			 }
		inline unsigned int VertexCount()	const { return vertex_count; }
		inline unsigned int IndexCount()	const { return index_count;  }
		inline bool			IsValid()		const { return vertex_count; } // Use the vertex count to implicitly define validness state.

	private:
		void CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexBufferLayout& vertex_buffer_layout );

	private:
		ID id;

		unsigned int vertex_buffer_id;
		unsigned int index_buffer_id;

		unsigned int vertex_count;
		unsigned int index_count;
	};
}
