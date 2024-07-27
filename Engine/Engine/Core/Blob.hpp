#pragma once

// std Includes.
#include <cstddef> // std::byte.
#include <vector>

namespace Engine
{
	class Blob
	{
	public:
		Blob() = default;
		Blob( const std::size_t size );

		/* Prohibit copying. */
		Blob( const Blob& rhs )				= delete;
		Blob& operator=( const Blob& rhs )	= delete;

		/* Allow moving. */
		Blob( Blob&& donor )				= default;
		Blob& operator=( Blob&& donor )		= default;

		~Blob() = default;

	/* Set/Get: */
		template< typename Type >
		void Set( const Type& value, const std::size_t offset )
		{
			WriteBytes( ( std::byte* )( &value ), offset, sizeof( Type ) );
		}

		void Set( const std::byte* value, const std::size_t offset, const std::size_t size );
		void* Get( const std::size_t offset );
		const void* Get( const std::size_t offset ) const;

		template< typename Type >
		Type& Get( const std::size_t offset )
		{
			auto pointer = ReadBytes( offset );
			return reinterpret_cast< Type& >( *( ( Type* )pointer ) );
		}

		template< typename Type >
		const Type& Get( const std::size_t offset ) const
		{
			const auto pointer = ReadBytes( offset );
			return reinterpret_cast< const Type& >( *( ( const Type* )pointer ) );
		}

	private:
		void WriteBytes( const std::byte* value, const std::size_t offset, const std::size_t size );
		void* ReadBytes( std::size_t offset );
		const void* ReadBytes( std::size_t offset ) const;

	protected:
		std::vector< std::byte > bytes;
	};
}
