// Engine Includes.
#include "Utility.h"

// std Includes.
#include <fstream>
#include <iostream>

namespace Engine
{
	namespace Utility
	{
		std::optional< std::string > ReadFileIntoString( const char* file_path, const char* optional_error_prompt )
		{
			std::ifstream file;
			file.exceptions( std::ifstream::failbit | std::ifstream::badbit );
			try
			{
				file.open( file_path );
				return std::string( ( std::istreambuf_iterator< char >( file ) ),
									( std::istreambuf_iterator< char >() ) );

			}
			catch( const std::ifstream::failure& e )
			{
				if( optional_error_prompt )
					std::cout << optional_error_prompt << "\n    " << e.what() << "\n";
				
				return std::nullopt;
			}
		}

		namespace String
		{
			std::string_view RemoveLeadingWhitespace( const std::string_view source )
			{
				return std::string_view( source.cbegin() + source.find_first_not_of( " \t" ), source.cend() );
			}

			std::string_view RemoveTrailingWhitespace( const std::string_view source )
			{
				if( source.back() == ' ' || source.back() == '\t' )
					return std::string_view( source.cbegin(), source.cbegin() + source.find_last_not_of( " \t" ) );

				return source; // No trailing whitespace found.
			}

			std::string_view FindPreviousWord( const std::string_view& source, const std::size_t offset )
			{
				const std::size_t last_char_pos = source.find_last_not_of( " \t", offset - 1 );

				if( last_char_pos != std::string::npos )
				{
					const std::size_t last_preceding_whitespace_pos = source.find_last_of( " \t", last_char_pos );

					if( last_preceding_whitespace_pos != std::string::npos )
					{
						return std::string_view( source.cbegin() + last_preceding_whitespace_pos + 1, source.begin() + last_char_pos + 1 );
					}

					// No whitespace found before the word.
					return std::string_view( source.cbegin(), source.begin() + last_char_pos + 1 );
				}

				return source; // Contains only white-space.
			}
		}
	}
}
