// Engine Includes.
#include "GLLogger.h"
#include "Texture.h"
#include "Core/ServiceLocator.h"

// Vendor/stb Includes.
#include "stb/stb_image.h"

// std Includes.
#include <iostream>

namespace Engine
{
	std::optional< Texture > Texture::Loader::FromFile( const::std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		int width, height;

		std::optional< Texture > maybe_texture;

		int number_of_channels = -1;
		auto image_data = stbi_load( file_path.c_str(), &width, &height, &number_of_channels, 4 );
		if( image_data )
			maybe_texture = Texture( name, ( std::byte* )image_data, import_settings.format, width, height,
									 import_settings.wrap_u, import_settings.wrap_v, import_settings.min_filter, import_settings.mag_filter );
		else
			std::cerr << "Could not load image data from file: \"" << file_path << "\"\n";

		stbi_image_free( image_data );
		
		return maybe_texture;
	}

	Texture::Texture()
		:
		id( -1 ),
		width( 0 ),
		height( 0 ),
		name( "<unnamed>" )
	{}

	Texture::Texture( const std::string_view name,
					  const std::byte* data,
					  const int format, const int width, const int height,
					  GLenum wrap_u, GLenum wrap_v,
					  GLenum min_filter, GLenum mag_filter )
		:
		id( -1 ),
		width( width ),
		height( height ),
		name( name )
	{
		glGenTextures( 1, &id );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id, this->name );
#endif // _DEBUG

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_u );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_v );

		/* NOTE: format parameter is not utilized. Internal format and the format parameters below are hard-coded for now. */
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	Texture::Texture( Texture&& donor )
		:
		id( std::exchange( donor.id, -1 ) ),
		name( std::exchange( donor.name, {} ) ),
		width( std::exchange( donor.width, 0 ) ),
		height( std::exchange( donor.height, 0 ) )
	{
	}

	Texture& Texture::operator =( Texture&& donor )
	{
		id     = std::exchange( donor.id, -1 );
		name   = std::exchange( donor.name, {} );
		width  = std::exchange( donor.width, 0 );
		height = std::exchange( donor.height, 0 );

		return *this;
	}

	Texture::~Texture()
	{
		glDeleteTextures( 1, &id );
	}

	void Texture::SetName( const std::string& new_name )
	{
		name = new_name;
	}

	void Texture::Activate( const int slot ) const
	{
		glActiveTexture( GL_TEXTURE0 + slot );
		Bind();
	}

	void Texture::Bind() const
	{
		glBindTexture( GL_TEXTURE_2D, id );
	}
}