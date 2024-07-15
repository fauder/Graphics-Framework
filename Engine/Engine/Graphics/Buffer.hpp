#pragma once

// Engine Includes.
#include "Graphics.h"

// std Includes.
#include <span>
#include <cstddef> // std::byte.

namespace Engine
{
	template< GLenum TargetType, typename BufferElementType >
	class Buffer
	{
	public:
		using ID = unsigned int;

	public:
		Buffer()
			:
			id( -1 ),
			count( 0 ),
			size( 0 )
		{
		}

		Buffer( const std::span< const BufferElementType > data_span, const GLenum usage = GL_STATIC_DRAW )
			:
			id( -1 ),
			count( ( unsigned int )data_span.size() ),
			size( ( unsigned int )data_span.size_bytes() )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'data_span' parameter passed to Buffer::Buffer< BufferElementType >( const std::span< BufferElementType > data_span, const GLenum usage ) is empty!" );

			CreateBuffer();
			Bind();
			Update( ( void* )data_span.data(), usage );
		}

		/* Prohibit copying; It does not make sense two have multiple Buffers with the same Id. */
		Buffer( const Buffer& other )			   = delete;
		Buffer& operator = ( const Buffer& other ) = delete;

		/* Allow moving. */
		Buffer( Buffer&& donor )
			:
			id( std::exchange( donor.id, -1 ) ),
			count( std::exchange( donor.count, 0 ) ),
			size( std::exchange( donor.size, 0 ) )
		{
		}
		
		Buffer& operator = ( Buffer&& donor )
		{
			id    = std::exchange( donor.id,	-1 );
			count = std::exchange( donor.count,  0 );
			size  = std::exchange( donor.size,	 0 );

			return *this;
		}
		
		~Buffer()
		{
			if( IsValid() )
				DeleteBuffer();
		}

		void Bind() const
		{
			ASSERT_DEBUG_ONLY( IsValid() && "Attempting Bind() on Buffer with zero size!" );

			GLCALL( glBindBuffer( TargetType, id ) );
		}

		void Update( const void* data, const GLenum usage ) const
		{
			Bind();

			GLCALL( glBufferData( TargetType, size, ( void* )data, usage ) );
		}

		void UpdatePartial( const std::span<std::byte> data_span, const std::byte offset_from_buffer_start, const GLenum usage ) const
		{
			Bind();

			GLCALL( glBufferSubData( TargetType, ( GLintptr )offset_from_buffer_start, data_span.size_bytes(), ( void* )data_span.data() ) );
		}
		
		ID				Id()		const { return id;		}
		unsigned int	Size()		const { return size;	}
		unsigned int	Count()		const { return count;	}
		bool			IsValid()	const { return size;	} // Use the size to implicitly define validness state.

	private:
		void CreateBuffer()
		{
			GLCALL( glGenBuffers( 1, &id ) );
		}

		void DeleteBuffer()
		{
			GLCALL( glDeleteBuffers( 1, &id ) );
		}

	private:
		ID id;
		unsigned int count;
		unsigned int size;
	};
}
