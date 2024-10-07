#pragma once

// std Includes.
#include <map>
#include <string>

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

		/* For assets with mulitple source-assets, such as cubemaps. */
		static AssetType* CreateAssetFromFile( const std::string& name, const std::initializer_list< std::string > file_paths,
											   const typename AssetType::ImportSettings& import_settings )
		{
			auto& instance = Instance();

			if( not instance.asset_path_map.contains( *file_paths.begin() ) ) // TODO: Fix this.
			{
				if( auto maybe_asset = AssetType::Loader::FromFile( name, file_paths, import_settings );
					maybe_asset )
				{
					instance.asset_map[ name ]      = std::move( *maybe_asset );
					instance.asset_path_map[ name ] = *file_paths.begin(); // TODO: Fix this.
				}
				else // Failed to load asset:
					return nullptr;
			}

			/* Asset is already loaded, return the existing one. */
			return &instance.asset_map[ name ];
		}

		static AssetType* CreateAssetFromMemory( const std::string& name, const std::byte* data, const int size, const typename AssetType::ImportSettings& import_settings )
		{
			auto& instance = Instance();

			if( name.empty() )
			{
				std::string new_name( "<unnamed>_" + std::to_string( ( int )instance.asset_map.size() ) );

				if( auto maybe_asset = AssetType::Loader::FromMemory( new_name, data, size, import_settings );
					maybe_asset )
				{
					instance.asset_map[ new_name ] = std::move( *maybe_asset );
					return &instance.asset_map[ new_name ];
				}

				// Failed to load asset:
				return nullptr;
			}
			else 
			{
				if( not instance.asset_map.contains( name ) ) // Can not compare file_paths as the asset does not & will not have a path.
				{
					if( auto maybe_asset = AssetType::Loader::FromMemory( name, data, size, import_settings );
						maybe_asset )
					{
						instance.asset_map[ name ] = std::move( *maybe_asset );
					}
					else // Failed to load asset:
						return nullptr;
				}

				/* Asset is already loaded, return the existing one. */
				return &instance.asset_map[ name ];
			}
		}

		static AssetType* AddExistingAsset( AssetType&& asset, const std::string& file_path = "<not-on-disk>" )
		{
			auto& instance = Instance();

			const auto& asset_name = asset.Name();

			if( instance.asset_map.contains( asset_name ) )
				return nullptr;

			instance.asset_path_map[ asset_name ] = file_path;
			return &( instance.asset_map[ asset_name ] = std::move( asset ) );
		}

		static AssetType* AddOrUpdateExistingAsset( AssetType&& asset, const std::string& file_path = "<not-on-disk>" )
		{
			auto& instance = Instance();

			const auto& asset_name = asset.Name();

			instance.asset_path_map[ asset_name ] = file_path;
			return &( instance.asset_map[ asset_name ] = std::move( asset ) );
		}

		static const std::map< std::string, AssetType >& Assets()
		{
			auto& instance = Instance();
			return instance.asset_map;
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
