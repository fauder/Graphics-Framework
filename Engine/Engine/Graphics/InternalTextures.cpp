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
		TEXTURE_MAP.try_emplace( "Normal Map", Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Default Normal Map", FullTexturePath( "default_normal_map.jpg" ),
																											  Engine::Texture::ImportSettings
																											  {
																												  .wrap_u = Engine::Texture::Wrapping::Repeat,
																												  .wrap_v = Engine::Texture::Wrapping::Repeat
																											  } ) );

		TEXTURE_MAP.try_emplace( "Missing", Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Missing", FullTexturePath( "missing_texture.jpg" ),
																										   Engine::Texture::ImportSettings
																										   {
																											   .wrap_u = Engine::Texture::Wrapping::Repeat,
																											   .wrap_v = Engine::Texture::Wrapping::Repeat
																										   } ) );
	}
}
