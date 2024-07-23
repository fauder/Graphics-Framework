// Engine Includes.
#include "GLLogger.h"
#include "Graphics.h"
#include "Texture.h"
#include "Core/ServiceLocator.h"

// Vendor/stb Includes.
#include "stb/stb_image.h"

// std Includes.
#include <iostream>

namespace Engine
{
	Texture::Texture()
		:
		id( -1 ),
		width( 0 ),
		height( 0 ),
		name( "<unnamed>" )
	{
	}

	Texture::Texture( const std::string_view& name )
		:
		id( -1 ),
		width( 0 ),
		height( 0 ),
		name( name )
	{
	}

	Texture::Texture( const std::string_view& name, const char* file_path, const int format )
		:
		id( -1 ),
		width( 0 ),
		height( 0 ),
		name( name )
	{
		FromFile( file_path, format );
	}

	Texture::~Texture()
	{
		glDeleteTextures( 1, &id );
	}

	void Texture::INITIALIZE()
	{
		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( true );
	}

	bool Texture::FromFile( const char* file_path, const int format )
	{
		glGenTextures( 1, &id );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id, name );
#endif // _DEBUG

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		int number_of_channels = -1;
		auto image_data = stbi_load( file_path, &width, &height, &number_of_channels, 0 );
		bool result = image_data;
		if( result )
		{
			glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_data );
			glGenerateMipmap( GL_TEXTURE_2D );
		}
		else
			std::cerr << "Could not load image data from file: \"" << file_path << "\"\n";

		stbi_image_free( image_data );

		return result;
	}

	void Texture::Activate( const int slot )
	{
		glActiveTexture( GL_TEXTURE0 + slot );
		Bind();
	}

	void Texture::Bind() const
	{
		glBindTexture( GL_TEXTURE_2D, id );
	}
}