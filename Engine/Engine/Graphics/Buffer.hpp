#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Core/Assertion.h"

// std Includes.
#include <span>
#include <cstddef> // std::byte.

// TODO: Keep track of buffer ids and debug/inspect them via ImGuiDrawer::Draw() or something.

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

		/* Only allocate memory.*/
		Buffer( const unsigned int size, const GLenum usage = GL_STATIC_DRAW )
			:
			id( -1 ),
			count( 0 ),
			size( size )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to Buffer::Buffer( const unsigned int size, const GLenum usage ) is empty!" );

			CreateBuffer();
			Bind();
			Update( nullptr, usage );
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

		/* To support cases where the count can not be deduced from the type alone;
		 * For example consider a vertex buffer that is passed as std::span< float >, with the following vertex attributes: 3x floats position, 3x floats normal, 2x floats uv.
		 * data_span.size() would give the actual vertex count times 8 (3 + 3 + 2) in this case. */
		Buffer( const unsigned int count, const std::span< const BufferElementType > data_span, const GLenum usage = GL_STATIC_DRAW )
			:
			id( -1 ),
			count( count ),
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

			glBindBuffer( TargetType, id );
		}

		void Update( const void* data, const GLenum usage = GL_STATIC_DRAW ) const
		{
			Bind();

			glBufferData( TargetType, size, data, usage );
		}

		void Update_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const
		{
			Bind();

			glBufferSubData( TargetType, ( GLintptr )offset_from_buffer_start, ( GLsizeiptr )data_span.size_bytes(), ( void* )data_span.data() );
		}
		
		ID				Id()		const { return id;		}
		unsigned int	Size()		const { return size;	}
		unsigned int	Count()		const { return count;	}
		bool			IsValid()	const { return size;	} // Use the size to implicitly define validness state.

	private:
		void CreateBuffer()
		{
			glGenBuffers( 1, &id );
		}

		void DeleteBuffer()
		{
			glDeleteBuffers( 1, &id );
		}

	private:
		ID id;
		unsigned int count;
		unsigned int size;
	};
}
