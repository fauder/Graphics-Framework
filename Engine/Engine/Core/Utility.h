#pragma once

// std Includes.
#include <optional>
#include <string>
#include <string_view>

namespace Engine
{
	namespace Utility
	{
		std::optional< std::string > ReadFileIntoString( const char* file_path, const char* optional_error_prompt = nullptr );

		namespace String
		{
			std::string_view RemoveLeadingWhitespace( const std::string_view source );
			std::string_view RemoveTrailingWhitespace( const std::string_view source );
			std::string_view FindPreviousWord( const std::string_view& source, const std::size_t offset );
		}
	};
}

