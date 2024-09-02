#pragma once

// Engine Includes.
#include "Mesh.h"
#include "Texture.h"

// std Includes
#include <optional>
#include <vector>

namespace Engine
{
	template< typename AssetType >
	class AssetDatabase;

	class Model
	{
	public:
		struct ImportSettings
		{
			GLenum usage;

			ImportSettings( const GLenum usage )
				:
				usage( usage )
			{}
		};

		struct Part // i.e., "sub-model".
		{
			std::vector< Mesh	 > sub_meshes;
			std::vector< Texture > textures;
		};

	private:
		class Loader
		{
		public:
			Loader( const Loader& ) = delete; 
			Loader& operator =( const Loader& ) = delete; 
			static std::optional< Model > FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings ); 
		private:
			Loader()
			{}

			static Loader& Instance()
			{
				static Loader instance;
				return instance;
			}
		};

		friend class AssetDatabase< Model >;
		friend class Loader;

	public:
		Model();
		Model( const std::string& name );

		/* Prevent copying for now: */
		Model( const Model& )			  = delete;
		Model& operator =( const Model& ) = delete;

		/* Allow moving: */
		Model( Model&& )			 = default;
		Model& operator =( Model&& ) = default;

		~Model();

	/* Queries: */
		inline int PartCount() const { return ( int )parts.size(); }
		inline const std::vector< Part >& Parts() const { return parts; }

	private:
		std::string name;

		std::vector< Part > parts;
	};
}
