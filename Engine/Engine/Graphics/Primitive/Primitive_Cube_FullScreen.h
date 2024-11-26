#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::NonIndexed::Cube_FullScreen
{
	constexpr std::array< Vector3, 36 > Positions
	( {
		{ -1.0f, -1.0f, -1.0f }, // FRONT
		{  1.0f,  1.0f, -1.0f }, // FRONT
		{  1.0f, -1.0f, -1.0f }, // FRONT
		{  1.0f,  1.0f, -1.0f }, // FRONT
		{ -1.0f, -1.0f, -1.0f }, // FRONT
		{ -1.0f,  1.0f, -1.0f }, // FRONT

		{ -1.0f, -1.0f,  1.0f }, // BACK
		{  1.0f, -1.0f,  1.0f }, // BACK
		{  1.0f,  1.0f,  1.0f }, // BACK
		{  1.0f,  1.0f,  1.0f }, // BACK
		{ -1.0f,  1.0f,  1.0f }, // BACK
		{ -1.0f, -1.0f,  1.0f }, // BACK

		{ -1.0f,  1.0f,  1.0f }, // LEFT
		{ -1.0f,  1.0f, -1.0f }, // LEFT
		{ -1.0f, -1.0f, -1.0f }, // LEFT
		{ -1.0f, -1.0f, -1.0f }, // LEFT
		{ -1.0f, -1.0f,  1.0f }, // LEFT
		{ -1.0f,  1.0f,  1.0f }, // LEFT

		{  1.0f,  1.0f,  1.0f }, // RIGHT
		{  1.0f, -1.0f, -1.0f }, // RIGHT
		{  1.0f,  1.0f, -1.0f }, // RIGHT
		{  1.0f, -1.0f, -1.0f }, // RIGHT
		{  1.0f,  1.0f,  1.0f }, // RIGHT
		{  1.0f, -1.0f,  1.0f }, // RIGHT

		{ -1.0f, -1.0f, -1.0f }, // BOTTOM
		{  1.0f, -1.0f, -1.0f }, // BOTTOM
		{  1.0f, -1.0f,  1.0f }, // BOTTOM
		{  1.0f, -1.0f,  1.0f }, // BOTTOM
		{ -1.0f, -1.0f,  1.0f }, // BOTTOM
		{ -1.0f, -1.0f, -1.0f }, // BOTTOM

		{ -1.0f,  1.0f, -1.0f }, // TOP
		{  1.0f,  1.0f,  1.0f }, // TOP
		{  1.0f,  1.0f, -1.0f }, // TOP
		{  1.0f,  1.0f,  1.0f }, // TOP
		{ -1.0f,  1.0f, -1.0f }, // TOP
		{ -1.0f,  1.0f,  1.0f }  // TOP
	} );

	constexpr std::array< Vector2, 36 > UVs
	( {
		{ 0.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 0.0f, 0.0f }, // FRONT
		{ 0.0f, 1.0f }, // FRONT

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
		{ 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f }, // RIGHT
		{ 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 0.0f }, // RIGHT
		{ 0.0f, 0.0f }, // RIGHT

		{ 0.0f, 1.0f }, // BOTTOM
		{ 1.0f, 1.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM
		{ 0.0f, 0.0f }, // BOTTOM
		{ 0.0f, 1.0f }, // BOTTOM

		{ 0.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f }, // TOP
		{ 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f }, // TOP
		{ 0.0f, 1.0f }, // TOP
		{ 0.0f, 0.0f }  // TOP
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
