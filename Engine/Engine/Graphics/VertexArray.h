#pragma once

// Engine Includes.
#include "VertexBuffer.h"
#include "VertexLayout.hpp"
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
		VertexArray( const std::string& name = {} );
		VertexArray( const VertexArray& right_hand_side )				= delete;
		VertexArray& operator =( const VertexArray& right_hand_side )	= delete;
		VertexArray( VertexArray&& donor );
		VertexArray& operator =( VertexArray&& donor );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout, const std::string& name = {} );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout, const IndexBuffer_U32& index_buffer, const std::string& name = {} );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout, const IndexBuffer_U16& index_buffer, const std::string& name = {} );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout, 
					 const std::optional< IndexBuffer_U16 >& index_buffer_u16, const std::optional< IndexBuffer_U32 >& index_buffer_u32,
					 const std::string& name = {} );
		~VertexArray();

		void Bind() const;
		void Unbind() const;

		inline ID			Id()			const { return id;			 }
		inline unsigned int VertexCount()	const { return vertex_count; }
		inline unsigned int IndexCount()	const { return index_count;  }

	private:
		inline bool IsValid() const { return vertex_count; } // Use the vertex count to implicitly define validness state.
		void CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout );

	private:
		ID id;

		std::string name;

		unsigned int vertex_buffer_id;
		unsigned int index_buffer_id;

		unsigned int vertex_count;
		unsigned int index_count;
	};
}
