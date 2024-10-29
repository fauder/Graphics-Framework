// Engine Includes.
#include "GLLogger.h"
#include "Texture.h"
#include "Core/ServiceLocator.h"
#include "Core/Assertion.h"

namespace Engine
{
/*
 * Utility functions:
 */

	int InternalFormat_sRGB( const int format )
	{
		switch( format )
		{
			case GL_RGB:	return GL_SRGB;
			case GL_RGBA:	return GL_SRGB_ALPHA;
			default:		return format;
		}
	};

/* 
 * Texture
 */

	Texture::Texture()
		:
		id( {} ),
		size( ZERO_INITIALIZATION ),
		type( TextureType::None ),
		name( "<unnamed>" ),
		sample_count( 0 ),
		is_sRGB() // Does not matter for default constructed Textures.
	{}

	/* Allocate-only constructor (no data).
	 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
	 */
	Texture::Texture( const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const int format,
					  const int width, const int height,
					  const bool is_sRGB,
					  Wrapping  wrap_u,		Wrapping  wrap_v,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D ),
		name( name ),
		sample_count( 0 ),
		is_sRGB( is_sRGB )
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

		glTexImage2D( GL_TEXTURE_2D, 0, is_sRGB ? InternalFormat_sRGB( format ) : format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr );

		/* No mip-map generation since there is no data yet. */

		Unbind();
	}

	/* Multi-sampled allocate-only constructor (no data).
	 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
	 */
	Texture::Texture( const int sample_count,
					  const std::string_view multi_sampled_texture_name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const int format,
					  const int width, const int height,
					  const bool is_sRGB )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D_MultiSample ),
		name( multi_sampled_texture_name ),
		sample_count( sample_count ),
		is_sRGB( is_sRGB )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name + " (" + std::to_string( sample_count ) + " samples) " );
#endif // _DEBUG

		glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sample_count, is_sRGB ? InternalFormat_sRGB( format ) : format, width, height, GL_TRUE );

		Unbind();
	}

	/* Cubemap allocate-only constructor (no data).
	 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
	 */
	Texture::Texture( CubeMapConstructorTag tag, 
					  const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const int format,
					  const int width, const int height,
					  const bool is_sRGB,
					  Wrapping  wrap_u,		Wrapping  wrap_v,	 Wrapping wrap_w,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Cubemap ),
		name( name ),
		sample_count( 0 ),
		is_sRGB( is_sRGB )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG


		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						  0, is_sRGB ? InternalFormat_sRGB( format ) : format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 ( GLenum )wrap_v );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 ( GLenum )wrap_w );

		Unbind();
	}

	Texture::Texture( Texture&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		size( std::exchange( donor.size, Vector2I{ ZERO_INITIALIZATION } ) ),
		type( std::exchange( donor.type, TextureType::None ) ),
		name( std::exchange( donor.name, {} ) ),
		sample_count( std::exchange( donor.sample_count, 0 ) )
		/* sRGB state does not matter. */
	{
	}

	Texture& Texture::operator =( Texture&& donor )
	{
		id           = std::exchange( donor.id,				{} );
		size         = std::exchange( donor.size,			Vector2I{ ZERO_INITIALIZATION } );
		type         = std::exchange( donor.type,			TextureType::None );
		name         = std::exchange( donor.name,			{} );
		sample_count = std::exchange( donor.sample_count,	0 );
		/* sRGB state does not matter. */

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
		ASSERT_DEBUG_ONLY( type == TextureType::Texture2D );

		glGenerateMipmap( ( GLenum )type );
	}

/*
 * TEXTURE PRIVATE API
 */

	/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data.
	 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
	 */
	Texture::Texture( const std::string_view name,
					  const std::byte* data,
					  const int format, const int width, const int height,
					  const bool is_sRGB,
					  Wrapping  wrap_u,		Wrapping  wrap_v,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D ),
		name( name ),
		sample_count( 0 ),
		is_sRGB( is_sRGB )
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

		glTexImage2D( GL_TEXTURE_2D, 0, is_sRGB ? InternalFormat_sRGB( format ) : format, width, height, 0, format, GL_UNSIGNED_BYTE, data );
		glGenerateMipmap( GL_TEXTURE_2D );
	
		Unbind();
	}

	/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data.
	 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
	 */
	Texture::Texture( CubeMapConstructorTag tag, 
					  const std::string_view name,
					  const std::array< const std::byte*, 6 >& cubemap_data_array,
					  const int format, const int width, const int height,
					  const bool is_sRGB,
					  Wrapping  wrap_u,		Wrapping  wrap_v,	Wrapping wrap_w,
					  Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Cubemap ),
		name( name ),
		sample_count( 0 ),
		is_sRGB( is_sRGB )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
						  0, is_sRGB ? InternalFormat_sRGB( format ) : format, width, height, 0, format, GL_UNSIGNED_BYTE, cubemap_data_array[ i ] );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 ( GLenum )wrap_v );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 ( GLenum )wrap_w );

		Unbind();
	}

	void Texture::Bind() const
	{
		glBindTexture( ( GLenum )type, id.Get() );
	}

	void Texture::Unbind() const
	{
		glBindTexture( ( GLenum )type, 0 );
	}
}