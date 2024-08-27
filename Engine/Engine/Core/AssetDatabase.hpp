#pragma once

// std Includes.
#include <map>
#include <string>

#include "Graphics/Texture.h"

namespace Engine
{
	// Singleton.
	template< typename AssetType >
	class AssetDatabase
	{
	public:
		AssetDatabase( AssetDatabase const& )			  = delete;
		AssetDatabase& operator =( AssetDatabase const& ) = delete;

		static AssetType* CreateAssetFromFile( const std::string& name, const std::string& file_path, const typename AssetType::ImportSettings& import_settings )
		{
			auto& instance = Instance();

			if( not instance.asset_path_map.contains( file_path ) )
			{
				if( auto maybe_asset = AssetType::Loader::FromFile( name, file_path, import_settings ); 
					maybe_asset )
				{
					instance.asset_map[ name ]      = std::move( *maybe_asset );
					instance.asset_path_map[ name ] = file_path;
				}
				else // Failed to load asset:
					return nullptr;
			}

			/* Asset is already loaded, return the existing one. */
			return &instance.asset_map[ name ];
		}

	private:
		AssetDatabase()
		{}

		static AssetDatabase& Instance()
		{
			static AssetDatabase instance;
			return instance;
		}

	private:
		// TODO: Implement & integrate GUID generation.

		std::map< std::string /* <-- Name (for now) */, AssetType > asset_map;
		std::map< std::string /* <-- Name (for now) */, std::string > asset_path_map;
	};
}
