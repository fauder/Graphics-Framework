// Engine Includes.
#include "Blob.hpp"

namespace Engine
{
	Blob::Blob()
	{
	}

	Blob::Blob( const std::size_t size )
		:
		bytes( size, std::byte{ 0 } )
	{
	}

	Blob::~Blob()
	{
	}

	void Blob::Set( const std::byte* value, const std::size_t offset, const std::size_t size )
	{
		WriteBytes( value, offset, size );
	}

	void* Blob::Get( const std::size_t offset )
	{
		return ReadBytes( offset );
	}

	const void* Blob::Get( const std::size_t offset ) const
	{
		return ReadBytes( offset );
	}

/* 
 * PRIVATE API
 */

	void Blob::WriteBytes( const std::byte* value, const std::size_t offset, const std::size_t size )
	{
		std::memcpy( bytes.data() + offset, value, size );
	}

	void* Blob::ReadBytes( std::size_t offset )
	{
		return bytes.data() + offset;
	}

	const void* Blob::ReadBytes( std::size_t offset ) const
	{
		return bytes.data() + offset;
	}
}