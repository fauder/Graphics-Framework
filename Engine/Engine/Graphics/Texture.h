#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Macros.h"
#include "Math/Vector.hpp"

// std Includes.
#include <optional>
#include <string>
#include <string_view>

namespace Engine
{
	template< typename AssetType >
	class AssetDatabase;

	enum class TextureType
	{
		None,

		Texture2D = GL_TEXTURE_2D,
		Cubemap   = GL_TEXTURE_CUBE_MAP
	};

	class Texture
	{
	public:
		struct CubeMapConstructorTag {};
		static constexpr CubeMapConstructorTag CUBEMAP_CONSTRUCTOR;

		enum class Wrapping
		{
			ClampToEdge       = GL_CLAMP_TO_EDGE,
			ClampToBorder     = GL_CLAMP_TO_BORDER,
			MirroredRepeat    = GL_MIRRORED_REPEAT,
			Repeat            = GL_REPEAT,
			MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
		};

		enum class Filtering
		{
			Nearest				  = GL_NEAREST,
			Linear			      = GL_LINEAR,
			Nearest_MipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
			Linear_MipmapNearest  = GL_LINEAR_MIPMAP_NEAREST,
			Nearest_MipmapLinear  = GL_NEAREST_MIPMAP_LINEAR,
			Linear_MipmapLinear	  = GL_LINEAR_MIPMAP_LINEAR
		};

		struct ImportSettings
		{
			int format;
			bool flip_vertically;

			Wrapping wrap_u;
			Wrapping wrap_v;
			Wrapping wrap_w;
			Filtering min_filter;
			Filtering mag_filter;

			ImportSettings( const int format = GL_RGBA, const bool flip_vertically = true,
							const Wrapping  wrap_u     = Wrapping::ClampToEdge,			 const Wrapping  wrap_v     = Wrapping::ClampToEdge,	const Wrapping wrap_w = Wrapping::ClampToEdge,
							const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear )
				:
				format( format ),
				flip_vertically( flip_vertically ),
				wrap_u( wrap_u ),
				wrap_v( wrap_v ),
				wrap_w( wrap_w ),
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
		/* Allocate-only constructor (no data). */
		Texture( const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const int format,
				 const int width, const int height,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v     = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Cubemap allocate-only constructor (no data). */
		Texture( CubeMapConstructorTag tag, const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const int format,
				 const int width, const int height,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v	 = Wrapping::ClampToEdge,	const Wrapping wrap_w = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Prevent copying for now: */
		Texture( const Texture& )				= delete;
		Texture& operator =( const Texture& )	= delete;

		/* Allow moving: */
		Texture( Texture&& );
		Texture& operator =( Texture&& );

		~Texture();

	/* Queries: */
		inline const ID				Id()		const { return id;			}
		inline const Vector2I&		Size()		const { return size;		}
		inline const int			Width()		const { return size.X();	}
		inline const int			Height()	const { return size.Y();	}
		inline const std::string&	Name()		const { return name;		}
		inline const TextureType	Type()		const { return type;		}

	/* Usage: */
		void SetName( const std::string& new_name );
		void Activate( const int slot ) const;
		void GenerateMipmaps();

	private:
		/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data. */
		Texture( const std::string_view name,
				 const std::byte* data,
				 const int format,
				 const int width, const int height,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v     = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data. */
		Texture( CubeMapConstructorTag tag, 
				 const std::string_view name,
				 const std::array< const std::byte*, 6 >& cubemap_data_array,
				 const int format,
				 const int width, const int height,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v     = Wrapping::ClampToEdge, const Wrapping wrap_w = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

	/* Queries: */
		bool IsValid() const { return size.X() > 0 && size.Y() > 0; } // Use the width & height to implicitly define validness state.

	/* Usage: */
		void Bind() const;

	private:
		ID id;
		Vector2I size;
		std::string name;
		TextureType type;
	};
};
