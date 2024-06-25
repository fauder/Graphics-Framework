#pragma once

// Engine Includes.
#include "Graphics/Color.hpp"
#include "Graphics/UniformStruct.h"
#include "Math/Vector.hpp"

namespace Engine::Lighting
{
	struct PointLightData : public UniformStruct
	{
		Color3 ambient, diffuse, specular;
		Vector3 position_view_space;

		float attenuation_constant, attenuation_linear, attenuation_quadratic;

	/* End of GLSL equivalence. */

		// Keeping world space position is beneficial; It is easier to reason about compared to view space coordinates.
		Vector3 position_world_space;
	};

	struct DirectionalLightData : public UniformStruct
	{
		Color3 ambient, diffuse, specular;
		Vector3 direction_view_space;

	/* End of GLSL equivalence. */

		// Keeping world space direction is beneficial; It is easier to reason about compared to view space coordinates.
		Vector3 direction_world_space;
	};

	struct SpotLightData : public UniformStruct
	{
		Color3 ambient, diffuse, specular;
		Vector3 position_view_space, direction_view_space; // Expected to be in view space when passed to shaders.
		float cos_cutoff_angle_inner, cos_cutoff_angle_outer;

	/* End of GLSL equivalence. */

		// Keeping world space position & direction is beneficial; It is easier to reason about compared to view space coordinates.
		Vector3 position_world_space, direction_world_space;
		Degrees cutoff_angle_inner, cutoff_angle_outer;
	};

	/* This is also named Material in some sources, but since the name Material is more widely used to define [Shader + Uniforms + Textures] instead, this struct is named SurfaceData */
	struct SurfaceData : public UniformStruct
	{
		int diffuse_map_slot, specular_map_slot;
		float shininess;
	};
}
