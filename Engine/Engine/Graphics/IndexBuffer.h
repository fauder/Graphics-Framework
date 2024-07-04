#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	class IndexBuffer
	{
	public:
		IndexBuffer();
		IndexBuffer( const unsigned int* index_data, const unsigned int size, const GLenum usage );

		/* Prohibit copying; It does not make sense two have multiple index buffers with the same Id. */
		IndexBuffer( const IndexBuffer& other )				 = delete;
		IndexBuffer& operator = ( const IndexBuffer& other ) = delete;

		/* Allow moving. */
		IndexBuffer( IndexBuffer&& donor );
		IndexBuffer& operator = ( IndexBuffer&& donor );

		~IndexBuffer();

		void Bind() const;

		inline unsigned int Size()		const { return size; }
		inline unsigned int Id()		const { return id;   }
		inline bool			IsValid()	const { return size; } // Use the size to implicitly define validness state.

	private:
		unsigned int id;
		unsigned int size;
	};
}
