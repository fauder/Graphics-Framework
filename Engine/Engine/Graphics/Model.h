#pragma once

// Engine Includes.
#include "Mesh.h"
#include "Texture.h"
#include "Math/Matrix.hpp"

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

		/* Maps to a glTF "primitive".
		 * It is a mesh with a unique material, i.e., a distinct draw call (unique in local MeshGroup, or glTF "mesh", not the whole model). */
		struct SubMesh
		{
			std::string name;
			Mesh& mesh; // Actual mesh storage is kept in the Model class.
			std::span< Texture* const > textures;
		};

		/* Maps to a glTF "mesh". 
		 * Does not containt a mesh directly; SubMeshes contained inside contain the actual Meshes.
		 * So basically, this is just a group. */
		struct MeshGroup
		{
			std::string name;
			std::vector< SubMesh > sub_meshes;
		};

		/* Same as a glTF "node". */
		struct Node
		{
			Node() = default;
			Node( const std::string& name, const Matrix4x4& transform_local, MeshGroup* mesh_group );

			/* Prevent copying but allow moving: */
			Node( const Node& )				= delete;
			Node& operator =( const Node& ) = delete;
			Node( Node&& );
			Node& operator =( Node&& );

			std::string name;
			Matrix4x4 transform_local;
			MeshGroup* mesh_group;
		};

	private:
		ASSET_LOADER_CLASS_DECLARATION_FROM_FILE_ONLY( Model );

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
		inline int NodeCount()		const { return ( int )nodes.size(); }
		inline int MeshCount()		const { return ( int )meshes.size(); }
		inline int MeshGroupCount() const { return ( int )mesh_groups.size(); }

		inline const std::vector< Node >& Nodes() const { return nodes; }
		inline const std::vector< Mesh >& Meshes() const { return meshes; }

	private:
		std::string name;

		std::vector< Node		> nodes;
		std::vector< MeshGroup	> mesh_groups;
		std::vector< Mesh		> meshes;
		std::vector< Texture*	> textures; // Storage of textures is kept by the AssetDatabase< Texture >.
	};
}
