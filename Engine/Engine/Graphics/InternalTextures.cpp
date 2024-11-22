// Engine Includes.
#include "InternalTextures.h"
#include "Renderer.h"
#include "Core/AssetDatabase.hpp"
#include "Core/Utility.hpp"
#include "Asset/Texture/InternalTextureDirectoryPath.h"


#define FullTexturePath( file_path ) std::string( Engine::TEXTURE_SOURCE_DIRECTORY_WITH_SEPARATOR ) + file_path
// Too lazy to get the above macro working constexpr. And frankly, unnecessary.

namespace Engine
{
	/* Static member variable definitions: */
	std::unordered_map< std::string, Texture* > InternalTextures::TEXTURE_MAP;

	Texture* InternalTextures::Get( const std::string& name )
	{
		// Just to get a better error message.
		ASSERT_DEBUG_ONLY( TEXTURE_MAP.contains( name ) && ( "Built-in texture with the name \"" + name + "\" was not found!" ).c_str() );

		return TEXTURE_MAP.find( name )->second;
	}

	void InternalTextures::Initialize()
	{
		TEXTURE_MAP.try_emplace( "Normal Map", AssetDatabase< Texture >::CreateAssetFromFile( "Default Normal Map", FullTexturePath( "default_normal_map.png" ),
																							  Texture::ImportSettings
																							  {
																								  .wrap_u     = Texture::Wrapping::Repeat,
																								  .wrap_v     = Texture::Wrapping::Repeat,
																								  .min_filter = Texture::Filtering::Nearest,
																								  .mag_filter = Texture::Filtering::Nearest,
																								  .is_sRGB    = false,
																							  } ) );

		TEXTURE_MAP.try_emplace( "Missing", AssetDatabase< Texture >::CreateAssetFromFile( "Missing", FullTexturePath( "missing_texture.jpg" ),
																						   Texture::ImportSettings
																						   {
																							   .wrap_u = Texture::Wrapping::Repeat,
																							   .wrap_v = Texture::Wrapping::Repeat
																						   } ) );
	}
}
