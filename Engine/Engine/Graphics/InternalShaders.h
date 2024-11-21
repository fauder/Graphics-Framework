#pragma once

// Engine Includes.
#include "Shader.hpp"

// std Includes.
#include <unordered_map>

namespace Engine
{
	/* Forward declarations: */
	class Renderer;

	class InternalShaders
	{
		friend class Renderer;

	public:
		static Shader* Get( const std::string& name );

	private:
		static void Initialize( Renderer& renderer );

	private:
		static std::unordered_map< std::string, Shader > SHADER_MAP;
	};
}
