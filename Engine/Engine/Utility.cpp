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
	}
}
