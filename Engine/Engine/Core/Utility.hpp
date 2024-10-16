#pragma once

// std Includes.
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Engine
{
	namespace Utility
	{
		std::optional< std::string > ReadFileIntoString( const char* file_path, const char* optional_error_prompt = nullptr );

		namespace String
		{
			std::string_view RemoveLeadingWhitespace( const std::string_view source );
			std::string_view RemoveTrailingWhitespace( const std::string_view source );
			std::string_view FindPreviousWord( const std::string_view source, const std::size_t offset );
			void Replace( std::string& source, const std::string_view find_this, const std::string_view replace_with_this );

			/* https://stackoverflow.com/a/75619411/4495751 */
			template< unsigned ... Length >
			constexpr auto ConstexprConcatenate( const char( &...strings )[ Length ] )
			{
				constexpr unsigned count = ( ... + Length ) - sizeof...( Length );
				std::array< char, count + 1 > result = {};
				result[ count ] = '\0';

				auto it = result.begin();
				( void )( ( it = std::copy_n( strings, Length - 1, it ), 0 ), ... );
				return result;
			}

			/* Returns either the multiple splitted string views or the source string in case delimiter was never found. */
			std::vector< std::string_view > Split( std::string_view source, const char delimiter );

#ifdef _WIN32
			std::wstring ToWideString( const std::string& string );
			std::string ToNarrowString( const std::wstring& wstring );
#endif // _WIN32
		}
	};
}
