#pragma once

// Engine Includes.
#include "Texture.h"

// std Includes.
#include <unordered_map>

namespace Engine
{
	/* Forward declarations: */
	class Renderer;

	/* Singleton. */
	class InternalTextures
	{
		friend class Renderer;

	public:
		static Texture* Get( const std::string& name );

	private:
		static void Initialize();

	private:
		static std::unordered_map< std::string, Texture* > TEXTURE_MAP;
	};
}
