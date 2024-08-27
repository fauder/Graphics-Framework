#pragma once

// Engine Includes.
#include "Uniform.h"
#include "UniformBuffer.h"

// std Includes.
#include <unordered_map>

namespace Engine
{
	// Singleton.
	class UniformBufferManager
	{
	public:
		UniformBufferManager( UniformBufferManager const& )				 = delete;
		UniformBufferManager& operator = ( UniformBufferManager const& ) = delete;

		static UniformBuffer* CreateOrRequest( const std::string& buffer_name, const Uniform::BufferInformation& buffer_info );

	private:
		UniformBufferManager();

		/* Singleton related: */
		static UniformBufferManager& Instance()
		{
			static UniformBufferManager instance;
			return instance;
		}

	private:
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_regular;
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_global;
		std::unordered_map< std::string, UniformBuffer > uniform_buffer_map_intrinsic;
	};
}
