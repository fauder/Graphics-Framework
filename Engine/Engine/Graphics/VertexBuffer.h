#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	class VertexBuffer
	{
	public:
		VertexBuffer( const void* vertex_data, const unsigned int size, const GLenum usage );
		~VertexBuffer();
		void Bind() const;
		unsigned int Size() const;

	private:
		unsigned int id;
		unsigned int size;
	};
}
