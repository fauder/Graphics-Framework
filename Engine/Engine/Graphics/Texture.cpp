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
/*
 * TEXTURE::LOADER API
 */

	std::optional< Texture > Texture::Loader::FromFile( const::std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		int width, height;

		std::optional< Texture > maybe_texture;

		int number_of_channels = -1;
		auto image_data = stbi_load( file_path.c_str(), &width, &height, &number_of_channels, 0 );
		if( image_data )
		{
			GLenum format = 0;

			switch( number_of_channels )
			{
				case 1: format = GL_RED;	break;
				case 2: format = GL_RG;		break;
				case 3: format = GL_RGB;	break;
				case 4: format = GL_RGBA;	break;
			}

			/* Format from import_settings is not used at the moment. */
			maybe_texture = Texture( name, ( std::byte* )image_data, format, width, height,
									 import_settings.wrap_u, import_settings.wrap_v, import_settings.min_filter, import_settings.mag_filter );
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

		int width, height;

		std::optional< Texture > maybe_texture;

		std::array< stbi_uc*, 6 > image_data_array;

		int number_of_channels = -1;

		for( auto i = 0; i < 6; i++ )
		{
			const auto& file_path( ( cubemap_file_paths.begin() + i ) );

			image_data_array[ i ] = stbi_load( file_path->c_str(), &width, &height, &number_of_channels, 0 );
			if( image_data_array[ i ] == nullptr )
			{
				std::cerr << "Could not load image data from file: \"" << file_path << "\"\n";
				stbi_image_free( image_data_array[ i ] );
				return maybe_texture;
			}
		}

		GLenum format = 0;

		switch( number_of_channels )
		{
			case 1: format = GL_RED;	break;
			case 2: format = GL_RG;		break;
			case 3: format = GL_RGB;	break;
			case 4: format = GL_RGBA;	break;
		}

		/* Format from import_settings is not used at the moment. */
		maybe_texture = Texture( CUBEMAP_CONSTRUCTOR, cubemap_name, ( const std::array< const std::byte*, 6 >& )image_data_array, format, width, height,
								 import_settings.wrap_u, import_settings.wrap_v, import_settings.wrap_w, import_settings.min_filter, import_settings.mag_filter );

		for( auto i = 0; i < 6; i++ )
			stbi_image_free( image_data_array[ i ] );
		
		return maybe_texture;
	}

	std::optional< Texture > Texture::Loader::FromMemory( const::std::string_view name, const std::byte* data, const int size, const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		int width, height;

		std::optional< Texture > maybe_texture;

		int number_of_channels = -1;
		auto image_data = stbi_load_from_memory( ( stbi_uc* )data, size, &width, &height, &number_of_channels, 0 );
		if( image_data )
		{
			GLenum format = 0;

			switch( number_of_channels )
			{
				case 1: format = GL_RED;	break;
				case 2: format = GL_RG;		break;
				case 3: format = GL_RGB;	break;
				case 4: format = GL_RGBA;	break;
			}

			/* Format from import_settings is not used at the moment. */
			maybe_texture = Texture( name, ( std::byte* )image_data, format, width, height,
									 import_settings.wrap_u, import_settings.wrap_v, import_settings.min_filter, import_settings.mag_filter );
		}
		else
			std::cerr << "Could not load image data from memory\n";

		stbi_image_free( image_data );
		
		return maybe_texture;
	}

/* 
 * TEXTURE PUBLIC API
 */

	Texture::Texture()
		:
		id( {} ),
		size( ZERO_INITIALIZATION ),
		name( "<unnamed>" ),
		type( TextureType::None )
	{}

	/* Allocate-only constructor (no data). */
	Texture::Texture( const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const int format, const int width, const int height,
					  Wrapping  wrap_u,		Wrapping  wrap_v,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		name( name ),
		type( TextureType::Texture2D )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	   ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	   ( GLenum )wrap_v );

		glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr );

		/* No mip-map generation since there is no data yet. */
	}

	/* Cubemap allocate-only constructor (no data). */
	Texture::Texture( CubeMapConstructorTag tag, 
					  const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const int format, const int width, const int height,
					  Wrapping  wrap_u,		Wrapping  wrap_v,	 Wrapping wrap_w,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		name( name ),
		type( TextureType::Cubemap )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG


		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 ( GLenum )wrap_v );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 ( GLenum )wrap_w );
	}

	Texture::Texture( Texture&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		name( std::exchange( donor.name, {} ) ),
		size( std::exchange( donor.size, Vector2I{ ZERO_INITIALIZATION } ) ),
		type( std::exchange( donor.type, TextureType::None ) )
	{
	}

	Texture& Texture::operator =( Texture&& donor )
	{
		id   = std::exchange( donor.id,		{} );
		name = std::exchange( donor.name,	{} );
		size = std::exchange( donor.size,	Vector2I{ ZERO_INITIALIZATION } );
		type = std::exchange( donor.type,	TextureType::None );

		return *this;
	}

	Texture::~Texture()
	{
		if( IsValid() )
		{
			glDeleteTextures( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
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

	void Texture::GenerateMipmaps()
	{
		Bind();
		glGenerateMipmap( GL_TEXTURE_2D );
	}

/*
 * TEXTURE PRIVATE API
 */

	/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data. */
	Texture::Texture( const std::string_view name,
					  const std::byte* data,
					  const int format, const int width, const int height,
					  Wrapping  wrap_u,		Wrapping  wrap_v,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		name( name ),
		type( TextureType::Texture2D )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	   ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	   ( GLenum )wrap_v );

		glTexImage2D( GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data );
		glGenerateMipmap( GL_TEXTURE_2D );
	}

	/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data. */
	Texture::Texture( CubeMapConstructorTag tag, 
					  const std::string_view name,
					  const std::array< const std::byte*, 6 >& cubemap_data_array,
					  const int format, const int width, const int height,
					  Wrapping  wrap_u,		Wrapping  wrap_v,	Wrapping wrap_w,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		name( name ),
		type( TextureType::Cubemap )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, cubemap_data_array[ i ] );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 ( GLenum )wrap_v );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 ( GLenum )wrap_w );
	}

	void Texture::Bind() const
	{
		glBindTexture( ( GLenum )type, id.Get() );
	}
}