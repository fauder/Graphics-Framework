#pragma once

// Engine Includes.
#include "Graphics.h"

// std Includes.
#include <vector>

namespace Engine
{
	struct VertexAttribute
	{
		unsigned int location;
		int count;
		GLenum type;
		bool normalized;

		inline unsigned int Size() const { return count * sizeof( type ); }
	};

	class VertexBufferLayout
	{
	public:
		template< typename Type >
		void Push( const int count );

		template <>
		void Push< int >( const int count )
		{
			attributes.push_back( { ( unsigned int )attributes.size(), count, GL_INT, GL_FALSE } );
		}

		template <>
		void Push< bool >( const int count )
		{
			attributes.push_back( { ( unsigned int )attributes.size(), count, GL_INT, GL_FALSE } );
		}

		template <>
		void Push< float >( const int count )
		{
			attributes.push_back( { ( unsigned int )attributes.size(), count, GL_FLOAT, GL_FALSE } );
		}

		inline void SetAndEnableAttributes() const
		{
			const unsigned int stride = Stride();
			unsigned int offset = 0;

			for( auto index = 0; index < attributes.size(); index++ )
			{
				const auto attribute = attributes[ index ];
				GLCALL( glVertexAttribPointer( attribute.location, attribute.count, attribute.type, attribute.normalized, stride, BUFFER_OFFSET( offset ) ) );
				GLCALL( glEnableVertexAttribArray( attribute.location ) );
				offset += attribute.Size();
			}
		}

		inline unsigned int Stride() const
		{
			unsigned int stride = 0;
			for( auto index = 0; index < attributes.size(); index++ )
				stride += attributes[ index ].Size();

			return stride;
		}

	private:
		std::vector< VertexAttribute > attributes;
	};
}
