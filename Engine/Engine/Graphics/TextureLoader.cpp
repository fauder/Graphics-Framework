// Engine Includes.
#include "Texture.h"

// Vendor/stb Includes.
#include "stb/stb_image.h"

// std Includes.
#include <array>
#include <iostream>

namespace Engine
{
	constexpr GLenum FORMAT        = GL_RGBA;
	constexpr int DESIRED_CHANNELS = 4;

	std::optional< Texture > Texture::Loader::FromFile( const::std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		std::optional< Texture > maybe_texture;

		int width, height;
		int number_of_channels = -1;
		auto image_data = stbi_load( file_path.c_str(), &width, &height, &number_of_channels, DESIRED_CHANNELS );
		if( image_data )
		{
			/* Format from import_settings is not used at the moment. */
			maybe_texture = Texture( name, 
									 ( std::byte* )image_data, 
									 FORMAT,
									 width, height, 
									 import_settings.is_sRGB,
									 import_settings.wrap_u, import_settings.wrap_v,
									 import_settings.border_color, 
									 import_settings.min_filter, import_settings.mag_filter );
		}
		else
			std::cerr << "Could not load image data from file: \"" << file_path << "\"\n";

		stbi_image_free( image_data );
		
		return maybe_texture;
	}

	std::optional< Texture > Texture::Loader::FromFile( const::std::string_view cubemap_name, const std::initializer_list< std::string > cubemap_file_paths,
														const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		/* OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		 *
		 * BUT: cube-map coordinate space has the inverse v behavior, so the image's should not be flipped. */
		stbi_set_flip_vertically_on_load( false ); // Override whatever is in import_settings.flip_vertically.

		std::optional< Texture > maybe_texture;

		std::array< stbi_uc*, 6 > image_data_array;

		int width, height;
		int number_of_channels = -1;

		for( auto i = 0; i < 6; i++ )
		{
			const auto& file_path( ( cubemap_file_paths.begin() + i ) );

			image_data_array[ i ] = stbi_load( file_path->c_str(), &width, &height, &number_of_channels, DESIRED_CHANNELS );
			if( image_data_array[ i ] == nullptr )
			{
				std::cerr << "Could not load image data from file: \"" << file_path << "\"\n";
				stbi_image_free( image_data_array[ i ] );
				return maybe_texture;
			}
		}

		maybe_texture = Texture( CUBEMAP_CONSTRUCTOR, 
								 cubemap_name, 
								 ( const std::array< const std::byte*, 6 >& )image_data_array, 
								 FORMAT, 
								 width, height,
								 import_settings.is_sRGB,
								 import_settings.wrap_u, import_settings.wrap_v, import_settings.wrap_w,
								 import_settings.border_color,
								 import_settings.min_filter, import_settings.mag_filter );

		for( auto i = 0; i < 6; i++ )
			stbi_image_free( image_data_array[ i ] );
		
		return maybe_texture;
	}

	std::optional< Texture > Texture::Loader::FromMemory( const::std::string_view name, const std::byte* data, const int size, const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		std::optional< Texture > maybe_texture;

		int width, height;
		int number_of_channels = -1;
		auto image_data = stbi_load_from_memory( ( stbi_uc* )data, size, &width, &height, &number_of_channels, DESIRED_CHANNELS );
		if( image_data )
		{
			/* Format from import_settings is not used at the moment. */
			maybe_texture = Texture( name, 
									 ( std::byte* )image_data, 
									 FORMAT, 
									 width, height, 
									 import_settings.is_sRGB,
									 import_settings.wrap_u, import_settings.wrap_v,
									 import_settings.border_color,
									 import_settings.min_filter, import_settings.mag_filter );
		}
		else
			std::cerr << "Could not load image data from memory\n";

		stbi_image_free( image_data );
		
		return maybe_texture;
	}
}
