#pragma once

// Engine Includes.
#include "Graphics/Color.hpp"
#include "Graphics/PaddedAndCombinedTypes.h"
#include "Graphics/Std140StructTag.h"
#include "Math/Vector.hpp"

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
}
