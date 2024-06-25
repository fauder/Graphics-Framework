#pragma once

// Engine Includes.
#include "Math/Vector.hpp"

namespace Engine
{
	// TODO: Convert to a proper class with actual camera functionality later.
	namespace Camera
	{
		Engine::Vector3 ConvertFromScreenSpaceToViewSpace( const Engine::Vector2 screen_space_coordinate, const Engine::Vector2I screen_dimensions, 
														   const float aspect_ratio, const float near_plane );
	}
}
