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

// std Includes.
#include <numeric>

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
	bool LoadMesh( fastgltf::Asset& gltf_asset, fastgltf::Mesh& gltf_mesh,
                   const Matrix4x4& local_transform, Model::MeshGroup& mesh_group_to_load, std::vector< Mesh >& meshes, const std::vector< Texture* >& textures )
    {
		/* Naming variables sub-mesh instead of gltf's "primitive" for better readibility. */

        mesh_group_to_load.sub_meshes.reserve( gltf_mesh.primitives.size() );
        mesh_group_to_load.name = gltf_mesh.name;

		for( auto submesh_iterator = gltf_mesh.primitives.begin(); submesh_iterator != gltf_mesh.primitives.end(); ++submesh_iterator )
        {
            std::size_t base_color_uv_index = 0;
            std::size_t material_index      = -1;

            std::span< Texture* const > sub_mesh_textures;

			auto* position_iterator = submesh_iterator->findAttribute( "POSITION" );
			ASSERT_DEBUG_ONLY( position_iterator != submesh_iterator->attributes.end() ); // A gltf mesh primitive is required to hold the POSITION attribute.

            if( submesh_iterator->materialIndex.has_value() )
            {
                material_index       = submesh_iterator->materialIndex.value();
                const auto& material = gltf_asset.materials[ submesh_iterator->materialIndex.value() ];

                const auto& base_color_texture_info = material.pbrData.baseColorTexture;
                if( base_color_texture_info.has_value() )
                {
                    const auto& fastgltf_texture = gltf_asset.textures[ base_color_texture_info->textureIndex ];
                    if( !fastgltf_texture.imageIndex.has_value() )
                        return false;

                    const auto albedo_texture = textures[ fastgltf_texture.imageIndex.value() ];
                    albedo_texture->SetName( "Albedo" );

                    sub_mesh_textures = std::span< Texture* const >( textures.data() + fastgltf_texture.imageIndex.value(), 1 );

                    if( base_color_texture_info->transform && base_color_texture_info->transform->texCoordIndex.has_value() )
                        base_color_uv_index = base_color_texture_info->transform->texCoordIndex.value();
                    else
                        base_color_uv_index = material.pbrData.baseColorTexture->texCoordIndex;
                }
            }

            /*
             * Positions:
             */

            /* glTF uses a right-handed coordinate system where x points to right, y points to up & z points from the screen to the user.
	         * Compared to the coordinate system used in this engine, only the Z component is the inverse, x & y are the same. */
	        static Matrix3x3 coordinate_system_transform( Vector3::Right(),
														  Vector3::Up(),
														  Vector3::Backward() );

			const auto& position_accessor = gltf_asset.accessors[ position_iterator->accessorIndex ];
			if( !position_accessor.bufferViewIndex.has_value() )
				continue;

            std::vector< Vector3 > positions( position_accessor.count, ZERO_INITIALIZATION );

            // TODO: Implement node hierarchy by chaining local transforms of nodes to get the global transform.

			fastgltf::iterateAccessorWithIndex< Vector3 >( gltf_asset, position_accessor,
														   [ & ]( Vector3 pos, std::size_t idx )
														   {
                                                    	   	   positions[ idx ] = ( Vector4( pos ).SetW( 1 ) * local_transform ).XYZ() * coordinate_system_transform;
														   } );

            /*
             * Normals:
             */

            std::vector< Vector3 > normals;

			if( auto* normal_iterator = submesh_iterator->findAttribute( "NORMAL" ); normal_iterator )
            {
                const auto& normal_accessor = gltf_asset.accessors[ normal_iterator->accessorIndex ];
                if( !normal_accessor.bufferViewIndex.has_value() )
                    continue;

                normals.resize( normal_accessor.count, ZERO_INITIALIZATION );

                fastgltf::copyFromAccessor< Vector3 >( gltf_asset, normal_accessor, normals.data() );
            }

            /*
             * UVs (0):
             */

            std::vector< Vector2 > uvs_0;

            if( auto* uvs_0_iterator = submesh_iterator->findAttribute( "TEXCOORD_0" ); uvs_0_iterator )
            {
                const auto& uvs_0_accessor = gltf_asset.accessors[ uvs_0_iterator->accessorIndex ];
                if( !uvs_0_accessor.bufferViewIndex.has_value() )
                    continue;

                uvs_0.resize( uvs_0_accessor.count, ZERO_INITIALIZATION );

                fastgltf::copyFromAccessor< Vector2 >( gltf_asset, uvs_0_accessor, uvs_0.data() );
            }

            /*
             * Indices:
             */

			ASSERT_DEBUG_ONLY( submesh_iterator->indicesAccessor.has_value() ); // We specify GenerateMeshIndices, so we should always have indices.

            const auto& index_accessor = gltf_asset.accessors[ submesh_iterator->indicesAccessor.value() ];
            if( !index_accessor.bufferViewIndex.has_value() )
                return false;
            const std::uint32_t index_count = static_cast< std::uint32_t >( index_accessor.count );

            std::vector< std::uint16_t > indices_u16;
            std::vector< std::uint32_t > indices_u32;

            if( index_accessor.componentType == fastgltf::ComponentType::UnsignedByte || index_accessor.componentType == fastgltf::ComponentType::UnsignedShort )
            {
                indices_u16.resize( index_count );
                fastgltf::copyFromAccessor< std::uint16_t >( gltf_asset, index_accessor, indices_u16.data() );
            }
            else
            {
                indices_u32.resize( index_count );
                fastgltf::copyFromAccessor< std::uint32_t >( gltf_asset, index_accessor, indices_u32.data() );
            }

            std::string sub_mesh_name( mesh_group_to_load.name + "_" + std::to_string( std::distance( gltf_mesh.primitives.begin(), submesh_iterator ) ) );

			mesh_group_to_load.sub_meshes.emplace_back( sub_mesh_name,
												        /* Actual Mesh will be stored inside the meshes vector. SubMesh will have a reference to this Mesh. */
														meshes.emplace_back( Mesh( std::move( positions ),
																				   sub_mesh_name,
																				   std::move( normals ),
																				   std::move( uvs_0 ),
																				   std::move( indices_u16 ), std::move( indices_u32 ) ) ),
														std::move( sub_mesh_textures ) );
		}

        return true;
    }

    bool LoadTexture( fastgltf::Asset& gltf_asset, fastgltf::Image& gltf_image,
                      Texture*& texture_to_load )
    {
        Engine::Texture::ImportSettings texture_import_settings;
        texture_import_settings.wrap_u = GL_MIRRORED_REPEAT;
        texture_import_settings.wrap_v = GL_MIRRORED_REPEAT;

        // TODO: Import sampler settings and remove the hard-coded wrapping parameters above.

        std::visit( fastgltf::visitor
                    {
                        []( auto& arg ) {},
                        [ & ]( fastgltf::sources::URI& file_path )
                        {
                            ASSERT_DEBUG_ONLY( file_path.fileByteOffset == 0 ); // Offsets with stbi are not supported.
                            ASSERT_DEBUG_ONLY( file_path.uri.isLocalPath() );   // Only capable of loading local files.

                            const std::string path( file_path.uri.path().begin(), file_path.uri.path().end() );

                            texture_to_load = AssetDatabase< Texture >::CreateAssetFromFile( std::string( gltf_image.name ), path, texture_import_settings );
                        },
                        [ & ]( fastgltf::sources::Array& vector )
                        {
                            texture_to_load = AssetDatabase< Texture >::CreateAssetFromMemory( std::string( gltf_image.name ), vector.bytes.data(), static_cast< int >( vector.bytes.size() ),
                                                                                       texture_import_settings );
                        },
                        [ & ]( fastgltf::sources::BufferView& view )
                        {
                            auto& buffer_view = gltf_asset.bufferViews[ view.bufferViewIndex ];
                            auto& buffer      = gltf_asset.buffers[ buffer_view.bufferIndex ];

                            /* Yes, we've already loaded every buffer into some GL buffer. 
                             * However, with GL it's simpler to just copy the buffer data again for the texture.
                             * Besides, this is just an example. */
                            std::visit( fastgltf::visitor
                                        {
                                            // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning all buffers are already loaded into a vector.
                                            []( auto& arg ) {},
                                            [ & ]( fastgltf::sources::Array& vector )
                                            {
                                                texture_to_load = AssetDatabase< Texture >::CreateAssetFromMemory( std::string( gltf_image.name ),
                                                                                                           vector.bytes.data() + buffer_view.byteOffset,
																										   static_cast< int >( buffer_view.byteLength ),
																										   texture_import_settings );
                                            }
                                        }, buffer.data );
                        },
                    }, gltf_image.data );

        return texture_to_load;
    }

    bool LoadNode( fastgltf::Asset& gltf_asset, fastgltf::Node& gltf_node,
                   std::vector< Model::MeshGroup >& mesh_groups, Model::Node& node_to_load )
    {
        const Matrix4x4 node_transform = std::visit( fastgltf::visitor
                                                     {
                                                         []( fastgltf::TRS& trs ) -> Matrix4x4
                                                         {
                                                             return
                                                                          Matrix::Scaling( reinterpret_cast< Vector3& >( trs.scale ) ) *
                                                                 Math::QuaternionToMatrix( reinterpret_cast< Quaternion& >( trs.rotation ) ) *
                                                                      Matrix::Translation( reinterpret_cast< Vector3& >( trs.translation ) );
                                                         },
                                                         []( fastgltf::math::fmat4x4& matrix ) -> Matrix4x4
                                                         {
                                                             // Need to transpose because fastgltf has column-major matrices.
                                                             return reinterpret_cast< Matrix4x4& >( matrix ).Transposed();
                                                         }
                                                     }, gltf_node.transform );

        node_to_load = Model::Node( std::string( gltf_node.name ), node_transform, gltf_node.meshIndex ? &mesh_groups[ *gltf_node.meshIndex ] : nullptr );

        return true;
    }

    std::optional< Model > Model::Loader::FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
        fastgltf::Asset gltf_asset;

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

            fastgltf::Expected< fastgltf::Asset > maybe_gltf_asset( fastgltf::Error::None );

            if( const auto gltf_type = fastgltf::determineGltfFileType( gltfFile.get() );
                gltf_type == fastgltf::GltfType::glTF )
                maybe_gltf_asset = parser.loadGltf( gltfFile.get(), path.parent_path(), gltf_options );
            else
                maybe_gltf_asset = parser.loadGltfBinary( gltfFile.get(), path.parent_path(), gltf_options );

            if( maybe_gltf_asset.error() != fastgltf::Error::None )
            {
                std::cerr << "ERROR::MODELLOADER::FASTGLTF::Failed to load glTF: " << fastgltf::getErrorMessage( maybe_gltf_asset.error() ) << '\n';
				return std::nullopt;
			}

            gltf_asset = std::move( maybe_gltf_asset.get() );
        }

        Model model( std::string{ name } );
        
        model.textures.reserve( gltf_asset.images.size() );

		for( auto& gltf_image : gltf_asset.images )
            if( not LoadTexture( gltf_asset, gltf_image, 
                                 model.textures.emplace_back() ) )
                return std::nullopt;

        /*
         * Mapping between the glTF & this engine: 
         * scene        -> ~Model (Default scene is assumed, multiple scenes are not supported).
         * node         -> Node
         * mesh         -> MeshGroup (Node has a MeshGroup pointer)
         * primitive    -> SubMesh (MeshGroup has a vector of SubMeshes).
         */

        model.mesh_groups.resize( gltf_asset.meshes.size() ); // Node points to an optional MeshGroup, MeshGroup vector has to be provided at this time.

        model.nodes.reserve( gltf_asset.nodes.size() );

        /* Weird gltf terminology; A "mesh" in glTF is actually just a *group*, whereas the "primitive" is the actual mesh, nested inside a "mesh".
         * A better name for a primitive would be sub-mesh.
         */

        const auto sub_mesh_count = std::accumulate( gltf_asset.meshes.cbegin(), gltf_asset.meshes.cend(), 0,
													 []( const int sum_so_far, const auto& gltf_mesh )
		                                             {
			                                             return sum_so_far + ( int )gltf_mesh.primitives.size();
		                                             } );

        model.meshes.reserve( sub_mesh_count );

        {
            int mesh_group_index = 0;

            for( auto& gltf_node : gltf_asset.nodes )
            {
                auto& node = model.nodes.emplace_back();
                if( not LoadNode( gltf_asset, gltf_node,
                                  model.mesh_groups, node ) )
                    return std::nullopt;

                if( node.mesh_group )
                {
                    if( not LoadMesh( gltf_asset, gltf_asset.meshes[ mesh_group_index ],
							          node.transform_local, *node.mesh_group, model.meshes, model.textures ) )
                        return std::nullopt;

                    mesh_group_index++;
                }
            }
        }

        return model;
	}
}