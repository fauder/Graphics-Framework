#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::NonIndexed::Cube
{
	constexpr std::array< Vector3, 36 > Positions
	( {
		{ -0.5f, -0.5f, -0.5f }, // FRONT
		{  0.5f, -0.5f, -0.5f }, // FRONT
		{  0.5f,  0.5f, -0.5f }, // FRONT
		{  0.5f,  0.5f, -0.5f }, // FRONT
		{ -0.5f,  0.5f, -0.5f }, // FRONT
		{ -0.5f, -0.5f, -0.5f }, // FRONT

		{ -0.5f, -0.5f,  0.5f }, // BACK
		{  0.5f, -0.5f,  0.5f }, // BACK
		{  0.5f,  0.5f,  0.5f }, // BACK
		{  0.5f,  0.5f,  0.5f }, // BACK
		{ -0.5f,  0.5f,  0.5f }, // BACK
		{ -0.5f, -0.5f,  0.5f }, // BACK

		{ -0.5f,  0.5f,  0.5f }, // LEFT
		{ -0.5f,  0.5f, -0.5f }, // LEFT
		{ -0.5f, -0.5f, -0.5f }, // LEFT
		{ -0.5f, -0.5f, -0.5f }, // LEFT
		{ -0.5f, -0.5f,  0.5f }, // LEFT
		{ -0.5f,  0.5f,  0.5f }, // LEFT

		{  0.5f,  0.5f,  0.5f }, // RIGHT
		{  0.5f,  0.5f, -0.5f }, // RIGHT
		{  0.5f, -0.5f, -0.5f }, // RIGHT
		{  0.5f, -0.5f, -0.5f }, // RIGHT
		{  0.5f, -0.5f,  0.5f }, // RIGHT
		{  0.5f,  0.5f,  0.5f }, // RIGHT

		{ -0.5f, -0.5f, -0.5f }, // BOTTOM
		{  0.5f, -0.5f, -0.5f }, // BOTTOM
		{  0.5f, -0.5f,  0.5f }, // BOTTOM
		{  0.5f, -0.5f,  0.5f }, // BOTTOM
		{ -0.5f, -0.5f,  0.5f }, // BOTTOM
		{ -0.5f, -0.5f, -0.5f }, // BOTTOM

		{ -0.5f,  0.5f, -0.5f }, // TOP
		{  0.5f,  0.5f, -0.5f }, // TOP
		{  0.5f,  0.5f,  0.5f }, // TOP
		{  0.5f,  0.5f,  0.5f }, // TOP
		{ -0.5f,  0.5f,  0.5f }, // TOP
		{ -0.5f,  0.5f, -0.5f }	 // TOP
	} );

	constexpr std::array< Vector2, 36 > UVs
	( {
		{ 0.0f, 0.0f }, // FRONT
		{ 1.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 0.0f, 1.0f }, // FRONT
		{ 0.0f, 0.0f }, // FRONT

		{ 0.0f, 0.0f }, // BACK
		{ 1.0f, 0.0f }, // BACK
		{ 1.0f, 1.0f }, // BACK
		{ 1.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f }, // BACK
		{ 0.0f, 0.0f }, // BACK

		{ 1.0f, 0.0f }, // LEFT
		{ 1.0f, 1.0f }, // LEFT
		{ 0.0f, 1.0f }, // LEFT
		{ 0.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f }, // LEFT
		{ 1.0f, 0.0f }, // LEFT

		{ 1.0f, 0.0f }, // RIGHT
		{ 1.0f, 1.0f }, // RIGHT
		{ 0.0f, 1.0f }, // RIGHT
		{ 0.0f, 1.0f }, // RIGHT
		{ 0.0f, 0.0f }, // RIGHT
		{ 1.0f, 0.0f }, // RIGHT

		{ 0.0f, 1.0f }, // BOTTOM
		{ 1.0f, 1.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM
		{ 0.0f, 0.0f }, // BOTTOM
		{ 0.0f, 1.0f }, // BOTTOM

		{ 0.0f, 1.0f }, // TOP
		{ 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f }, // TOP
		{ 1.0f, 0.0f }, // TOP
		{ 0.0f, 0.0f }, // TOP
		{ 0.0f, 1.0f }  // TOP
	} );

	constexpr std::array< Vector2, 36 > UVs_CubeMap
	( {
		{ 1.0f,  0.333f	}, // FRONT
		{ 0.75f, 0.333f	}, // FRONT
		{ 0.75f, 0.666f	}, // FRONT
		{ 0.75f, 0.666f	}, // FRONT
		{ 1.0f,  0.666f	}, // FRONT
		{ 1.0f,  0.333f	}, // FRONT

		{ 0.25f, 0.333f	}, // BACK
		{ 0.5f,  0.333f	}, // BACK
		{ 0.5f,  0.666f	}, // BACK
		{ 0.5f,  0.666f	}, // BACK
		{ 0.25f, 0.666f	}, // BACK
		{ 0.25f, 0.333f	}, // BACK

		{ 0.25f, 0.666f	}, // LEFT
		{ 0.0f,  0.666f	}, // LEFT
		{ 0.0f,  0.333f	}, // LEFT
		{ 0.0f,  0.333f	}, // LEFT
		{ 0.25f, 0.333f	}, // LEFT
		{ 0.25f, 0.666f	}, // LEFT

		{ 0.5f,  0.666f	}, // RIGHT
		{ 0.75f, 0.666f	}, // RIGHT
		{ 0.75f, 0.333f	}, // RIGHT
		{ 0.75f, 0.333f	}, // RIGHT
		{ 0.5f,  0.333f	}, // RIGHT
		{ 0.5f,  0.666f	}, // RIGHT

		{ 0.25f, 0.0f	}, // BOTTOM
		{ 0.5f,  0.0f	}, // BOTTOM
		{ 0.5f,  0.333f	}, // BOTTOM
		{ 0.5f,  0.333f	}, // BOTTOM
		{ 0.25f, 0.333f	}, // BOTTOM
		{ 0.25f, 0.0f	}, // BOTTOM

		{ 0.25f, 1.0f	}, // TOP
		{ 0.5f,  1.0f	}, // TOP
		{ 0.5f,  0.666f	}, // TOP
		{ 0.5f,  0.666f	}, // TOP
		{ 0.25f, 0.666f	}, // TOP
		{ 0.25f, 1.0f	}  // TOP
	} );

	constexpr std::array< Vector3, 36 > Normals
	( {
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT

		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK

		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT

		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT

		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM

		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }  // TOP
	} );

	constexpr std::array< Vector4, 36 > VertexColors
	( {
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT

		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK

		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT

		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT

		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM

		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }  // TOP
	} );
}
