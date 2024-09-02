#pragma once

// Engine Includes.
#include "Buffer.hpp"

// std Includes.
#include <variant>

namespace Engine
{
	using IndexBuffer_U16 = Buffer< GL_ELEMENT_ARRAY_BUFFER, std::uint16_t >;
	using IndexBuffer_U32 = Buffer< GL_ELEMENT_ARRAY_BUFFER, std::uint32_t >;
}
