// Engine Includes.
#include "UniformBufferManagement.hpp"
#include "UniformBufferBindingPointManager.h"
#include "UniformBufferManager.h"

namespace Engine
{
	void UniformBufferManagement::RegisterBuffer( const std::string& buffer_name, const Uniform::BufferInformation* buffer_info )
	{
		if( buffer_info_map.try_emplace( buffer_name, buffer_info ).second ) // .second returns whether the emplace was successfull or not.
		{
			buffer_map.emplace( buffer_name, UniformBufferManager::CreateOrRequest( buffer_name, *buffer_info ) );

			blob_map.emplace( buffer_name, buffer_info->size );
		}
	}

	void UniformBufferManagement::UnregisterBuffer( const std::string& buffer_name )
	{
		if( const auto& iterator = buffer_info_map.find( buffer_name );
			iterator != buffer_info_map.cend() )
		{
			buffer_map.erase( buffer_name );
			buffer_info_map.erase( buffer_name );
			blob_map.erase( buffer_name );
		}
	}

	void UniformBufferManagement::UnregisterAllBuffers()
	{
		if( not buffer_info_map.empty() )
		{
			buffer_info_map.clear();
			blob_map.clear();
		}
	}

	const void* UniformBufferManagement::Get( const std::string& buffer_name ) const
	{
		return blob_map.at( buffer_name ).Get( 0 );
	}

	void* UniformBufferManagement::Get( const std::string& buffer_name )
	{
		return blob_map[ buffer_name ].Get( 0 );
	}

	void UniformBufferManagement::UploadAll()
	{
		for( auto& [ uniform_buffer_name, uniform_blob ] : blob_map )
		{
			const auto& uniform_buffer      = buffer_map[ uniform_buffer_name ];
			const auto& uniform_buffer_info = buffer_info_map[ uniform_buffer_name ];

			if( uniform_blob.IsDirty() )
			{
				uniform_blob.MergeConsecutiveDirtySections();
				const auto& dirty_sections = uniform_blob.DirtySections();
				for( auto& dirty_section : dirty_sections )
					uniform_buffer->Update_Partial( uniform_blob.SpanFromSection( dirty_section ), dirty_section.offset );

				uniform_blob.ClearDirtySections();
			}
		}
	}
}
