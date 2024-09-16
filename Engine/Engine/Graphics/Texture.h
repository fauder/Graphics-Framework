#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Macros.h"

// std Includes.
#include <string>
#include <string_view>

namespace Engine
{
	template< typename AssetType >
	class AssetDatabase;

	class Texture
	{
	public:
		struct ImportSettings
		{
			int format;
			bool flip_vertically;

			GLenum wrap_u;
			GLenum wrap_v;
			GLenum min_filter;
			GLenum mag_filter;

			ImportSettings( const int format = GL_RGBA, const bool flip_vertically = true,
							GLenum wrap_u = GL_CLAMP_TO_EDGE, GLenum wrap_v = GL_CLAMP_TO_EDGE,
							GLenum min_filter = GL_LINEAR_MIPMAP_LINEAR, GLenum mag_filter = GL_LINEAR )
				:
				format( format ),
				flip_vertically( flip_vertically ),
				wrap_u( wrap_u ),
				wrap_v( wrap_v ),
				min_filter( min_filter ),
				mag_filter( mag_filter )
			{}
		};

	private:
		friend class AssetDatabase< Texture >;
		
		ASSET_LOADER_CLASS_DECLARATION( Texture );

	public:
		using ID = unsigned int;

	public:
		Texture();

		/* Prevent copying for now: */
		Texture( const Texture& )				= delete;
		Texture& operator =( const Texture& )	= delete;

		/* Allow moving: */
		Texture( Texture&& );
		Texture& operator =( Texture&& );

		~Texture();

	/* Queries: */
		inline const ID				Id()	const { return id;	 }
		inline const std::string&	Name()	const { return name; }
		void SetName( const std::string& new_name );

	/* Usage: */
		void Activate( const int slot ) const;

	private:
	/* Private constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with parameters. */
		Texture( const std::string_view name,
				 const std::byte* data,
				 const int format,
				 const int width, const int height,
				 GLenum wrap_u, GLenum wrap_v,
				 GLenum min_filter, GLenum mag_filter );

	/* Queries: */
		bool IsValid() const { return width > 0 && height > 0; } // Use the width & height to implicitly define validness state.

	/* Usage: */
		void Bind() const;

	private:
		ID id;
		int width, height;
		std::string name;
	};
};
