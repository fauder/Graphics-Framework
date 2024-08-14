// Engine Includes.
#include "UniformBufferManager.h"
#include "UniformBufferBindingPointManager.h"
#include "Core/Optimization.h"

namespace Engine
{
	UniformBuffer* UniformBufferManager::CreateOrRequest( const std::string& buffer_name, const Uniform::BufferInformation& buffer_info )
	{
		auto& instance = Instance();

		switch( buffer_info.category )
		{
			case Uniform::BufferCategory::Regular:
			{
				auto& buffer = instance.uniform_buffer_map_regular.try_emplace( buffer_name, buffer_info.size, buffer_name ).first->second;
				UniformBufferBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Regular );

				return &buffer;
			}
			case Uniform::BufferCategory::Instance:
			{
				throw std::runtime_error( "Instance Uniforms not implemented yet!" );

				auto& buffer = instance.uniform_buffer_map_instance.try_emplace( buffer_name, buffer_info.size, buffer_name ).first->second;
				UniformBufferBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Instance );

				return &buffer;
			}
			case Uniform::BufferCategory::Global:
			{
				auto& buffer = instance.uniform_buffer_map_global.try_emplace( buffer_name, buffer_info.size, buffer_name ).first->second;
				UniformBufferBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Global );

				return &buffer;
			}
			case Uniform::BufferCategory::Intrinsic:
			{
				auto& buffer = instance.uniform_buffer_map_intrinsic.try_emplace( buffer_name, buffer_info.size, buffer_name ).first->second;
				UniformBufferBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Intrinsic );

				return &buffer;
			}
		}

		UNREACHABLE();
	}

/*
 *
 * PRIVATE API:
 *
 */

	UniformBufferManager::UniformBufferManager()
	{
	}
}
