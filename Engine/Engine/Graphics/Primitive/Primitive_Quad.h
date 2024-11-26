#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::NonIndexed::Quad
{
	constexpr std::array< Vector3, 6 > Positions
	( {
		{ -0.5f, -0.5f, 0.0f }, // FRONT
		{  0.5f,  0.5f, 0.0f }, // FRONT
		{  0.5f, -0.5f, 0.0f }, // FRONT
		{  0.5f,  0.5f, 0.0f }, // FRONT
		{ -0.5f, -0.5f, 0.0f }, // FRONT
		{ -0.5f,  0.5f, 0.0f }  // FRONT
	} );

	constexpr std::array< Vector2, 6 > UVs
	( {
		{ 0.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 0.0f, 0.0f }, // FRONT
		{ 0.0f, 1.0f }  // FRONT
	} );

	constexpr std::array< Vector3, 6 > Normals
	( {
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }  // FRONT
	} );

	constexpr std::array< Vector4, 6 > VertexColors
	( {
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }  // FRONT
	} );
}
