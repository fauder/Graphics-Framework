#pragma once

// std Includes.
#include <optional>
#include <string>

namespace Engine
{
	namespace Utility
	{
		std::optional< std::string > ReadFileIntoString( const char* file_path, const char* optional_error_prompt = nullptr );
	};
}

