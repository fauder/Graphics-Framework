#pragma once

// Engine Includes.
#include "Blob.hpp"

// std Includes.
#include <span>

namespace Engine
{
	class DirtyBlob : public Blob
	{
	public:
		struct Section
		{
			std::size_t offset, size;

			constexpr auto operator<=>( const Section& ) const = default;
		};

	public:
		DirtyBlob() = default;
		DirtyBlob( const std::size_t size );

		/* Prohibit copying. */
		DirtyBlob( const DirtyBlob& rhs )				= delete;
		DirtyBlob& operator=( const DirtyBlob& rhs )	= delete;

		/* Allow moving. */
		DirtyBlob( DirtyBlob&& donor )				= default;
		DirtyBlob& operator=( DirtyBlob&& donor )	= default;

		~DirtyBlob() = default;

	/* Set/Get: */
		template< typename Type >
		void Set( const Type& value, const std::size_t offset )
		{
			Blob::Set( ( std::byte* )( &value ), offset, sizeof( Type ) );

			dirty_sections.push_back( Section{ offset, sizeof( Type ) } );
		}

		void Set( const std::byte* value, const std::size_t offset, const std::size_t size );

	/* Allocation/Deallocation: */
		void Clear();
		
	/* Dirty API: */
		inline bool IsDirty() const { return not dirty_sections.empty(); }
		void MergeConsecutiveDirtySections();
		void ClearDirtySections();
		inline const std::vector< Section >& DirtySections() const { return dirty_sections; }
		std::span< std::byte > SpanFromSection( const Section& section );

	private:
		std::vector< Section > dirty_sections;
	};
}
