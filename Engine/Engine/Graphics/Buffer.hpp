#pragma once

// Engine Includes.
#include "GLLogger.h"
#include "Graphics.h"
#include "ID.hpp"
#include "Core/ServiceLocator.h"
#include "Core/Assertion.h"

// std Includes.
#include <span>
#include <string>
#include <cstddef> // std::byte.

// TODO: Keep track of buffer ids and debug/inspect them via ImGuiDrawer::Draw() or something.

namespace Engine
{
	template< GLenum TargetType >
	class Buffer
	{
	public:
		using ID = ID< Buffer >;

	public:
		Buffer()
			:
			id( ID( 0 ) ),
			name(),
			count( 0 ),
			size( 0 )
		{
		}

		/* Only allocate memory.*/
		Buffer( const unsigned int size, const std::string& name = {}, const GLenum usage = GL_STATIC_DRAW )
			:
			id( ID( 0 ) ),
			name( name ),
			count( 0 ),
			size( size )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'size' parameter passed to Buffer::Buffer( const unsigned int size, const GLenum usage ) is empty!" );

			CreateBuffer();
			Bind();
			Update( nullptr, usage );

#ifdef _DEBUG
			if( not name.empty() )
				ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.Get(), name );
#endif // _DEBUG
		}

		template< typename BufferElementType >
		Buffer( const std::span< BufferElementType > data_span, const std::string& name = {}, const GLenum usage = GL_STATIC_DRAW )
			:
			id( ID( 0 ) ),
			name( name ),
			count( ( unsigned int )data_span.size() ),
			size( ( unsigned int )data_span.size_bytes() )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'data_span' parameter passed to Buffer::Buffer< BufferElementType >( const std::span< BufferElementType > data_span, const GLenum usage ) is empty!" );

			CreateBuffer();
			Bind();
			Update( ( void* )data_span.data(), usage );

#ifdef _DEBUG
			if( not name.empty() )
				ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.Get(), name );
#endif // _DEBUG
		}

		/* To support cases where the count can not be deduced from the type alone;
		 * For example consider a vertex buffer that is passed as std::span< float >, with the following vertex attributes: 3x floats position, 3x floats normal, 2x floats uv.
		 * data_span.size() would give the actual vertex count times 8 (3 + 3 + 2) in this case. */
		template< typename BufferElementType >
		Buffer( const unsigned int count, const std::span< BufferElementType > data_span, const std::string& name = {}, const GLenum usage = GL_STATIC_DRAW )
			:
			id( ID( 0 ) ),
			name( name ),
			count( count ),
			size( ( unsigned int )data_span.size_bytes() )
		{
			ASSERT_DEBUG_ONLY( IsValid() && "'data_span' parameter passed to Buffer::Buffer< BufferElementType >( const std::span< BufferElementType > data_span, const GLenum usage ) is empty!" );

			CreateBuffer();
			Bind();
			Update( ( void* )data_span.data(), usage );

#ifdef _DEBUG
			if( not name.empty() )
				ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.Get(), name );
#endif // _DEBUG
		}

		/* Prohibit copying; It does not make sense two have multiple Buffers with the same Id. */
		Buffer( const Buffer& other )			  = delete;
		Buffer& operator =( const Buffer& other ) = delete;

		/* Allow moving. */
		Buffer( Buffer&& donor )
			:
			id( std::exchange( donor.id, {} ) ),
			name( std::exchange( donor.name, {} ) ),
			count( std::exchange( donor.count, 0 ) ),
			size( std::exchange( donor.size, 0 ) )
		{
		}
		
		Buffer& operator =( Buffer&& donor )
		{
			id    = std::exchange( donor.id,	{} );
			name  = std::exchange( donor.name,	{} );
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

			glBindBuffer( TargetType, id.Get() );
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
		
		const ID&		Id()	const { return id;		}
		unsigned int	Size()	const { return size;	}
		unsigned int	Count()	const { return count;	}

	private:
		bool IsValid() const { return size;	} // Use the size to implicitly define validness state.

		void CreateBuffer()
		{
			glGenBuffers( 1, id.Address() );
		}

		void DeleteBuffer() 
		{
			glDeleteBuffers( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}

	private:
		ID id;
		std::string name;
		unsigned int count;
		unsigned int size;
	};

	using VertexBuffer   = Buffer< GL_ARRAY_BUFFER >;
	using InstanceBuffer = Buffer< GL_ARRAY_BUFFER >;
	using IndexBuffer    = Buffer< GL_ELEMENT_ARRAY_BUFFER >;
	using UniformBuffer  = Buffer< GL_UNIFORM_BUFFER >;
}
