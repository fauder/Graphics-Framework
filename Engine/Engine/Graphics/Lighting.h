#pragma once

// Engine Includes.
#include "Graphics/Color.hpp"
#include "Graphics/UniformStruct.h"
#include "Math/Vector.hpp"

namespace Engine::Lighting
{
	struct LightData : public UniformStruct
	{
		Color3 ambient, diffuse, specular;
		Vector3 position;
	};

	/* This is also named Material in some sources, but since the name Material is more widely used to define [Shader + Uniforms + Textures] instead, this struct is named SurfaceData */
	struct SurfaceData : public UniformStruct
	{
		Color3 ambient, diffuse, specular;
		float shininess;
	};
}
