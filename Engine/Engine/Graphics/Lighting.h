#pragma once

// Engine Includes.
#include "Color.hpp"
#include "Math/Vector.hpp"
#include "PaddedAndCombinedTypes.h"
#include "Std140StructTag.h"

namespace Engine::Lighting
{
	struct DirectionalLightData : public Std140StructTag
	{
		Color3_Padded ambient, diffuse, specular;
		Vector3_Padded direction_view_space;

	/* End of GLSL equivalence. C++ only extras: */

		// Keeping world space direction is beneficial; It is easier to reason about compared to view space coordinates.
		Vector3 direction_world_space;
	};

	struct PointLightData : public Std140StructTag
	{
		Color3_AndScalar ambient_and_attenuation_constant, diffuse_and_attenuation_linear, specular_attenuation_quadratic;
		Vector3_Padded position_view_space;

	/* End of GLSL equivalence. C++ only extras: */

		// Keeping world space position is beneficial; It is easier to reason about compared to view space coordinates.
		Vector3 position_world_space;
	};

	struct SpotLightData : public Std140StructTag
	{
		Color3_Padded ambient, diffuse, specular;
		Vector3_AndScalar position_view_space_and_cos_cutoff_angle_inner, direction_view_space_and_cos_cutoff_angle_outer; // Expected to be in view space when passed to shaders.

	/* End of GLSL equivalence. C++ only extras: */

		// Keeping world space position & direction is beneficial; It is easier to reason about compared to view space coordinates.
		Vector3 position_world_space, direction_world_space;
		Degrees cutoff_angle_inner, cutoff_angle_outer;
	};

	/* This is also named Material in some sources, but since the name Material is more widely used to define [Shader + Uniforms + Textures] instead, this struct is named SurfaceData */
	struct SurfaceData : public Std140StructTag
	{
		float shininess;
	};
}
