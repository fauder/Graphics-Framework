// Engine Includes.
#include "Model.h"

// Vendor Includes.
#pragma warning(disable:5223)
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#pragma warning(default:5223)

template <>
struct fastgltf::ElementTraits< Engine::Vector2  > : fastgltf::ElementTraitsBase< Engine::Vector2,  AccessorType::Vec2, float> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3  > : fastgltf::ElementTraitsBase< Engine::Vector3,  AccessorType::Vec3, float> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4  > : fastgltf::ElementTraitsBase< Engine::Vector4,  AccessorType::Vec4, float> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector2D > : fastgltf::ElementTraitsBase< Engine::Vector2D, AccessorType::Vec2, double> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3D > : fastgltf::ElementTraitsBase< Engine::Vector3D, AccessorType::Vec3, double> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4D > : fastgltf::ElementTraitsBase< Engine::Vector4D, AccessorType::Vec4, double> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector2U > : fastgltf::ElementTraitsBase< Engine::Vector2U, AccessorType::Vec2, unsigned int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3U > : fastgltf::ElementTraitsBase< Engine::Vector3U, AccessorType::Vec3, unsigned int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4U > : fastgltf::ElementTraitsBase< Engine::Vector4U, AccessorType::Vec4, unsigned int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector2I > : fastgltf::ElementTraitsBase< Engine::Vector2I, AccessorType::Vec2, int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3I > : fastgltf::ElementTraitsBase< Engine::Vector3I, AccessorType::Vec3, int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4I > : fastgltf::ElementTraitsBase< Engine::Vector4I, AccessorType::Vec4, int> {};

namespace Engine
{
	bool LoadMesh( fastgltf::Asset& asset, std::vector< Model::Part >& model_parts, fastgltf::Mesh& mesh )
    {
		/* Naming variables sub-mesh instead of gltf's "primitive" for better readibility. */

        auto& model_part = model_parts.emplace_back();
        model_part.sub_meshes.reserve( mesh.primitives.size() );

		for( auto submesh_iterator = mesh.primitives.begin(); submesh_iterator != mesh.primitives.end(); ++submesh_iterator )
        {
            /*
             * POSITIONS:
             */

			auto* position_iterator = submesh_iterator->findAttribute( "POSITION" );
			ASSERT_DEBUG_ONLY( position_iterator != submesh_iterator->attributes.end() ); // A mesh primitive is required to hold the POSITION attribute.

			const auto& position_accessor = asset.accessors[ position_iterator->accessorIndex ];
			if( !position_accessor.bufferViewIndex.has_value() )
				continue;

            std::vector< Vector3 > positions( position_accessor.count, ZERO_INITIALIZATION );

            /* Leaving this here for quick reference; Example code for iterating an accessor with an index. Useful when whole vertex data resides in a single buffer (i.e., already interleaved). */
			/*fastgltf::iterateAccessorWithIndex< Vector3 >( asset, position_accessor,
															[ & ]( Vector3 pos, std::size_t idx )
															{
																positions[ idx ] = pos;
															} );*/

            fastgltf::copyFromAccessor< Vector3 >( asset, position_accessor, positions.data() );

            /*
             * INDICES:
             */

			ASSERT_DEBUG_ONLY( submesh_iterator->indicesAccessor.has_value() ); // We specify GenerateMeshIndices, so we should always have indices.

            const auto& index_accessor = asset.accessors[ submesh_iterator->indicesAccessor.value() ];
            if( !index_accessor.bufferViewIndex.has_value() )
                return false;
            const std::uint32_t index_count = static_cast< std::uint32_t >( index_accessor.count );

            std::vector< std::uint16_t > indices_u16;
            std::vector< std::uint32_t > indices_u32;

            if( index_accessor.componentType == fastgltf::ComponentType::UnsignedByte || index_accessor.componentType == fastgltf::ComponentType::UnsignedShort )
            {
                indices_u16.resize( index_count );
                fastgltf::copyFromAccessor< std::uint16_t >( asset, index_accessor, indices_u16.data() );
            }
            else
            {
                indices_u32.resize( index_count );
                fastgltf::copyFromAccessor< std::uint32_t >( asset, index_accessor, indices_u32.data() );
            }

            std::vector< Vector3 > normals( position_accessor.count, ZERO_INITIALIZATION );
            std::vector< Vector2 > uvs_0( position_accessor.count, ZERO_INITIALIZATION );

            model_part.sub_meshes.emplace_back( std::move( positions ),
                                                std::string{},
                                                std::move( normals ),
                                                std::move( uvs_0 ),
                                                std::move( indices_u16 ), std::move( indices_u32 ) );
        }

        return true;
    }

    std::optional< Model > Model::Loader::FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
        fastgltf::Asset asset;

        // Parse the glTF file and get the constructed asset:
        {
            static constexpr auto supported_extensions =
                fastgltf::Extensions::KHR_mesh_quantization |
                fastgltf::Extensions::KHR_texture_transform |
                fastgltf::Extensions::KHR_materials_variants;

            fastgltf::Parser parser( supported_extensions );

            constexpr auto gltf_options =
                fastgltf::Options::DontRequireValidAssetMember |
                fastgltf::Options::AllowDouble |
                fastgltf::Options::LoadExternalBuffers |
                fastgltf::Options::LoadExternalImages |
                fastgltf::Options::GenerateMeshIndices;

			auto gltfFile = fastgltf::GltfDataBuffer::FromPath( file_path );
			if( !bool( gltfFile ) )
            {
				std::cerr << "ERROR::MODELLOADER::FASTGLTF::Failed to open glTF file: " << fastgltf::getErrorMessage( gltfFile.error() ) << '\n';
				return std::nullopt;
			}

            std::filesystem::path path( file_path );

            fastgltf::Expected< fastgltf::Asset > maybe_asset( fastgltf::Error::None );

            if( const auto gltf_type = fastgltf::determineGltfFileType( gltfFile.get() );
                gltf_type == fastgltf::GltfType::glTF )
			    maybe_asset = parser.loadGltf( gltfFile.get(), path.parent_path(), gltf_options );
            else
                maybe_asset = parser.loadGltfBinary( gltfFile.get(), path.parent_path(), gltf_options );

			if( maybe_asset.error() != fastgltf::Error::None )
            {
				std::cerr << "ERROR::MODELLOADER::FASTGLTF::Failed to load glTF: " << fastgltf::getErrorMessage( maybe_asset.error() ) << '\n';
				return std::nullopt;
			}

            asset = std::move( maybe_asset.get() );
        }

        Model model( std::string{ name } );
        model.parts.reserve( asset.meshes.size() ); // LoadMesh() will emplace_back() to avoid having to pass the index.
        auto& model_parts = model.parts;

		/* Weird gltf terminology; A *mesh* is actually just a *group*, whereas the "primitive" is the actual mesh, nested inside a "mesh". A better name for a primitive would be sub-mesh. */

        for( auto& mesh : asset.meshes )
            if( not LoadMesh( asset, model_parts, mesh ) )
                return std::nullopt;

        return model;
	}
}
