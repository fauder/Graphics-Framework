#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	class VertexBuffer
	{
	public:
		VertexBuffer();
		VertexBuffer( const void* vertex_data, const unsigned int size, const GLenum usage );

		/* Prohibit copying; It does not make sense two have multiple vertex buffers with the same Id. */
		VertexBuffer( const VertexBuffer& other )			   = delete;
		VertexBuffer& operator = ( const VertexBuffer& other ) = delete;

		/* Allow moving. */
		VertexBuffer( VertexBuffer&& donor );
		VertexBuffer& operator = ( VertexBuffer&& donor );
		
		~VertexBuffer();
		
		void Bind() const;
		
		inline unsigned int Size()		const { return size; }
		inline unsigned int Id()		const { return id;   }
		inline bool			IsValid()	const { return size; } // Use the size to implicitly define validness state.

	private:
		unsigned int id;
		unsigned int size;
	};
}
