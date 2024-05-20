#pragma once

// Engine Includes.
#include "Graphics.h"

namespace Engine
{
	class IndexBuffer
	{
	public:
		IndexBuffer( const unsigned int* index_data, const unsigned int size, const GLenum usage );
		~IndexBuffer();
		void Bind() const;
		unsigned int Size() const;

	private:
		unsigned int id;
		unsigned int size;
	};
}
