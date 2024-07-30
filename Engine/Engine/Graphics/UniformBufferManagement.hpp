#pragma once

// Engine Includes.
#include "Std140StructTag.h"
#include "Uniform.h"
#include "UniformBuffer.h"
#include "Core/DirtyBlob.h"

// std Includes.
#include <unordered_map>

namespace Engine
{
	class UniformBufferManagement
	{
	public:
		UniformBufferManagement()  = default;

		/* Prevent copying & moving: */
		UniformBufferManagement( const UniformBufferManagement& other )				= delete;
		UniformBufferManagement& operator=( const UniformBufferManagement& other )	= delete;
		UniformBufferManagement( UniformBufferManagement&& donor )					= delete;
		UniformBufferManagement& operator=( UniformBufferManagement&& donor )		= delete;

		~UniformBufferManagement() = default;

	/* Register/Unregister Buffer API: */
		void RegisterBuffer( const std::string& buffer_name, const Uniform::BufferInformation* buffer_info );
		void UnregisterBuffer( const std::string& buffer_name );
		void UnregisterAllBuffers();

	/* Uniform Set/Get: */
		const void* Get( const std::string& buffer_name, const Uniform::BufferInformation& buffer_info ) const;
			  void* Get( const std::string& buffer_name, const Uniform::BufferInformation& buffer_info );

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const std::string& buffer_name, const StructType& value )
		{
			const auto& buffer_info = *buffer_info_map[ buffer_name ];

			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), 0 /* because every buffer has its own blob. */, buffer_info.size);
		}

		/* For PARTIAL setting ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const std::string& buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			const auto& buffer_info              = *buffer_info_map[ buffer_name ];
			const auto& buffer_member_array_info = buffer_info.members_array_map.at( uniform_member_array_instance_name );

			const auto effective_offset = buffer_member_array_info.offset + 
										  array_index * buffer_member_array_info.stride;

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, buffer_member_array_info.stride );
		}

		/* For PARTIAL setting STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const std::string& buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			const auto& buffer_info               = *buffer_info_map[ buffer_name ];
			const auto& buffer_member_struct_info = buffer_info.members_struct_map.at( uniform_member_struct_instance_name );

			const auto effective_offset = buffer_member_struct_info.offset;

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, buffer_member_struct_info.size );
		}

		/* For PARTIAL setting NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType >
		void Set( const std::string& buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			const auto& buffer_info               = *buffer_info_map[ buffer_name ];
			const auto& buffer_member_single_info = buffer_info.members_single_map.at( uniform_member_name );

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), buffer_member_single_info->offset, buffer_member_single_info->size );
		}

	/* Uniform Upload: */
		void UploadAll();

	private:
		// TODO: Use hashes instead of strings as keys.

		std::unordered_map< std::string, const Uniform::BufferInformation* > buffer_info_map;

		std::unordered_map< std::string, UniformBuffer > buffer_map;

		std::unordered_map< std::string, DirtyBlob > blob_map;
	};
}
