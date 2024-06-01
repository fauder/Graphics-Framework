#pragma once

namespace Engine
{
	namespace Initialization
	{
		struct UniformInitialization	{};
		struct ZeroInitialization		{};
		struct NoInitialization			{};
	}

	static constexpr Initialization::UniformInitialization	UNIFORM_INITIALIZATION;
	static constexpr Initialization::ZeroInitialization		ZERO_INITIALIZATION;
	static constexpr Initialization::NoInitialization		NO_INITIALIZATION;
}