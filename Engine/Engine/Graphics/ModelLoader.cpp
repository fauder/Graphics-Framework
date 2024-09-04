// Engine Includes.
#include "Model.h"
#include "Core/AssetDatabase.hpp"
#include "Math/Matrix.h"
#include "Math/Quaternion.hpp"

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
	bool LoadMesh( fastgltf::Asset& asset, std::vector< Model::Part >& model_parts, const std::vector< Texture* >& textures, fastgltf::Mesh& mesh )
    {
		/* Naming variables sub-mesh instead of gltf's "primitive" for better readibility. */

        auto& model_part = model_parts.emplace_back();
        model_part.sub_meshes.reserve( mesh.primitives.size() );
        model_part.name = mesh.name;

		for( auto submesh_iterator = mesh.primitives.begin(); submesh_iterator != mesh.primitives.end(); ++submesh_iterator )
        {
            std::size_t base_color_uv_index = 0;
            std::size_t material_index      = -1;

			auto* position_iterator = submesh_iterator->findAttribute( "POSITION" );
			ASSERT_DEBUG_ONLY( position_iterator != submesh_iterator->attributes.end() ); // A mesh primitive is required to hold the POSITION attribute.

            if( submesh_iterator->materialIndex.has_value() )
            {
                material_index = submesh_iterator->materialIndex.value();
                const auto& material = asset.materials[ submesh_iterator->materialIndex.value() ];

                const auto& base_color_texture_info = material.pbrData.baseColorTexture;
                if( base_color_texture_info.has_value() )
                {
                    const auto& fastgltf_texture = asset.textures[ base_color_texture_info->textureIndex ];
                    if( !fastgltf_texture.imageIndex.has_value() )
                        return false;

                    const auto albedo_texture = textures[ fastgltf_texture.imageIndex.value() ];
                    albedo_texture->SetName( "Albedo" );

                    model_part.textures.push_back( albedo_texture );

                    if( base_color_texture_info->transform && base_color_texture_info->transform->texCoordIndex.has_value() )
                        base_color_uv_index = base_color_texture_info->transform->texCoordIndex.value();
                    else
                        base_color_uv_index = material.pbrData.baseColorTexture->texCoordIndex;
                }
            }

            /*
             * Positions:
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
             * Normals:
             */

            std::vector< Vector3 > normals;

			if( auto* normal_iterator = submesh_iterator->findAttribute( "NORMAL" ); normal_iterator )
            {
                const auto& normal_accessor = asset.accessors[ normal_iterator->accessorIndex ];
                if( !normal_accessor.bufferViewIndex.has_value() )
                    continue;

                normals.resize( normal_accessor.count, ZERO_INITIALIZATION );

                fastgltf::copyFromAccessor< Vector3 >( asset, normal_accessor, normals.data() );
            }

            /*
             * UVs (0):
             */

            std::vector< Vector2 > uvs_0;

            if( auto* uvs_0_iterator = submesh_iterator->findAttribute( "TEXCOORD_0" ); uvs_0_iterator )
            {
                const auto& uvs_0_accessor = asset.accessors[ uvs_0_iterator->accessorIndex ];
                if( !uvs_0_accessor.bufferViewIndex.has_value() )
                    continue;

                uvs_0.resize( uvs_0_accessor.count, ZERO_INITIALIZATION );

                fastgltf::copyFromAccessor< Vector2 >( asset, uvs_0_accessor, uvs_0.data() );
            }

            /*
             * Indices:
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

            model_part.sub_meshes.emplace_back( std::move( positions ),
                                                model_part.name + "_" + std::to_string( std::distance( mesh.primitives.begin(), submesh_iterator ) ),
                                                std::move( normals ),
                                                std::move( uvs_0 ),
                                                std::move( indices_u16 ), std::move( indices_u32 ) );
        }

        return true;
    }

    bool LoadTexture( fastgltf::Asset& asset, std::vector< Texture* >& textures, fastgltf::Image& image )
    {
        Engine::Texture::ImportSettings texture_import_settings;
        texture_import_settings.wrap_u = GL_MIRRORED_REPEAT;
        texture_import_settings.wrap_v = GL_MIRRORED_REPEAT;

        // TODO: Import sampler settings and remove the hard-coded wrapping parameters above.

        Texture* texture;
        
        std::visit( fastgltf::visitor
                    {
                        []( auto& arg ) {},
                        [ & ]( fastgltf::sources::URI& file_path )
                        {
                            ASSERT_DEBUG_ONLY( file_path.fileByteOffset == 0 ); // Offsets with stbi are not supported.
                            ASSERT_DEBUG_ONLY( file_path.uri.isLocalPath() );   // Only capable of loading local files.

                            const std::string path( file_path.uri.path().begin(), file_path.uri.path().end() );

                            texture = AssetDatabase< Texture >::CreateAssetFromFile( std::string( image.name ), path, texture_import_settings );
                        },
                        [ & ]( fastgltf::sources::Array& vector )
                        {
                            texture = AssetDatabase< Texture >::CreateAssetFromMemory( std::string( image.name ), vector.bytes.data(), static_cast< int >( vector.bytes.size() ),
                                                                                       texture_import_settings );
                        },
                        [ & ]( fastgltf::sources::BufferView& view )
                        {
                            auto& buffer_view = asset.bufferViews[ view.bufferViewIndex ];
                            auto& buffer      = asset.buffers[ buffer_view.bufferIndex ];

                            /* Yes, we've already loaded every buffer into some GL buffer. 
                             * However, with GL it's simpler to just copy the buffer data again for the texture.
                             * Besides, this is just an example. */
                            std::visit( fastgltf::visitor
                                        {
                                            // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning all buffers are already loaded into a vector.
                                            []( auto& arg ) {},
                                            [ & ]( fastgltf::sources::Array& vector )
                                            {
												texture = AssetDatabase< Texture >::CreateAssetFromMemory( std::string( image.name ),
                                                                                                           vector.bytes.data() + buffer_view.byteOffset,
																										   static_cast< int >( buffer_view.byteLength ),
																										   texture_import_settings );
                                            }
                                        }, buffer.data );
                        },
                    }, image.data );

        if( texture )
        {
            textures.push_back( texture );
            return true;
        }

        return false;
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

        std::vector< Texture* > textures;

        for( auto& image : asset.images )
            if( not LoadTexture( asset, textures, image ) )
                return std::nullopt;

		/* Weird gltf terminology; A *mesh* is actually just a *group*, whereas the "primitive" is the actual mesh, nested inside a "mesh". A better name for a primitive would be sub-mesh. */

        for( auto& mesh : asset.meshes )
            if( not LoadMesh( asset, model_parts, textures, mesh ) )
                return std::nullopt;

        return model;
	}
}
