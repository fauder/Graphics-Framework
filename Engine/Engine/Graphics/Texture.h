#pragma once

// Engine Includes.
#include "Graphics.h"
#include "GraphicsMacros.h"
#include "ID.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <string>
#include <string_view>

namespace Engine
{
	template< typename AssetType >
	class AssetDatabase;

	enum class TextureType
	{
		None,

		Texture2D             = GL_TEXTURE_2D,
		Texture2D_MultiSample = GL_TEXTURE_2D_MULTISAMPLE,
		Cubemap               = GL_TEXTURE_CUBE_MAP
	};

	class Texture
	{
		struct CubeMapConstructorTag {};
		struct Texture2DMultiSampleConstructorTag {};

	public:
		static constexpr CubeMapConstructorTag CUBEMAP_CONSTRUCTOR = {};
		static constexpr Texture2DMultiSampleConstructorTag TEXTURE_2D_MULTISAMPLE_CONSTRUCTOR = {};

		enum class Wrapping // Has to be a default enum (4 bytes), because the GLenums are all over the place...
		{
			ClampToEdge       = GL_CLAMP_TO_EDGE,
			ClampToBorder     = GL_CLAMP_TO_BORDER,
			MirroredRepeat    = GL_MIRRORED_REPEAT,
			Repeat            = GL_REPEAT,
			MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
		};

		enum class Filtering // Has to be a default enum (4 bytes), because the GLenums are all over the place...
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
			int format = GL_RGBA;

			Wrapping wrap_u = Wrapping::ClampToEdge;
			Wrapping wrap_v = Wrapping::ClampToEdge;
			Wrapping wrap_w = Wrapping::ClampToEdge;

			Filtering min_filter = Filtering::Linear_MipmapLinear;
			Filtering mag_filter = Filtering::Linear;

			bool flip_vertically = true;
			bool is_sRGB         = true;
			//bool padding[ 2 ];
		};

		static constexpr ImportSettings DEFAULT_IMPORT_SETTINGS = {};

	private:
		friend class AssetDatabase< Texture >;
		
		ASSET_LOADER_CLASS_DECLARATION( Texture );

	public:
		using ID = ID< Texture >;

	public:
		Texture();
		/* Allocate-only constructor (no data).
		 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
		 */
		Texture( const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const int format,
				 const int width, const int height,
				 const bool is_sRGB = true,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v	 = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Multi-sampled allocate-only constructor (no data).
		 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
		 */
		Texture( const int sample_count,
				 const std::string_view multi_sample_texture_name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const int format,
				 const int width, const int height,
				 const bool is_sRGB = true );

		/* Cubemap allocate-only constructor (no data).
		 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
		 */
		Texture( CubeMapConstructorTag tag,
				 const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const int format,
				 const int width, const int height,
				 const bool is_sRGB = true,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v	 = Wrapping::ClampToEdge,	const Wrapping wrap_w = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		DELETE_COPY_CONSTRUCTORS( Texture );

		/* Allow moving: */
		Texture( Texture&& );
		Texture& operator =( Texture&& );

		~Texture();

	/* Queries: */
		bool IsValid() const { return id.Get(); }

		inline ID					Id()				const { return id;				}
		inline const Vector2I&		Size()				const { return size;			}
		inline int					Width()				const { return size.X();		}
		inline int					Height()			const { return size.Y();		}
		inline TextureType			Type()				const { return type;			}
		inline const std::string&	Name()				const { return name;			}
		inline int					SampleCount()		const { return sample_count;	}
		inline bool					IsMultiSampled()	const { return sample_count;	}

	/* Usage: */
		void SetName( const std::string& new_name );
		void Activate( const int slot ) const;
		void GenerateMipmaps();

	private:
		/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data.
		 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
		 */
		Texture( const std::string_view name,
				 const std::byte* data,
				 const int format,
				 const int width, const int height,
				 const bool is_sRGB = true,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v     = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data.
		 * Parameter 'is_sRGB': Set this to true for albedo/diffuse maps, false for normal maps etc. (for linear color space textures).
		 */
		Texture( CubeMapConstructorTag tag, 
				 const std::string_view name,
				 const std::array< const std::byte*, 6 >& cubemap_data_array,
				 const int format,
				 const int width, const int height,
				 const bool is_sRGB = true,
				 const Wrapping  wrap_u     = Wrapping::ClampToEdge,		  const Wrapping  wrap_v     = Wrapping::ClampToEdge, const Wrapping wrap_w = Wrapping::ClampToEdge,
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		void Delete();

	/* Usage: */
		void Bind() const;
		void Unbind() const;

	private:
		ID id;
		Vector2I size;
		TextureType type;
		std::string name;
		int sample_count;
		bool is_sRGB;
		// bool padding[ 3 ];
	};
};
