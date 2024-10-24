// Project Includes.
#include "ModelInstance.h"

ModelInstance::ModelInstance()
	:
	model( nullptr )
{
}

ModelInstance::ModelInstance( const Engine::Model* model, Engine::Shader* const shader,
							  const Vector3 scale, const Quaternion& rotation, const Vector3& translation,
							  const Engine::Texture* diffuse_texture, const Engine::Texture* specular_texture,
							  const Vector4 texture_scale_and_offset,
							  const std::initializer_list< std::pair< Engine::Renderer::RenderGroupID, Engine::Material* > > render_group_material_info_list )
	:
	model( model )
{
	ASSERT_DEBUG_ONLY( model != nullptr );

	SetMaterialData( shader, diffuse_texture, specular_texture, texture_scale_and_offset );

	/* Initialize Transforms, Materials & Drawables of Nodes: */

	const auto mesh_instance_count = model->MeshInstanceCount();
	const auto& meshes             = model->Meshes();
	const auto& nodes              = model->Nodes();

	node_transform_array.resize( mesh_instance_count );

	ASSERT_DEBUG_ONLY( render_group_material_info_list.size() >= 1 );

	for( auto render_group_index = 0; render_group_index < render_group_material_info_list.size(); render_group_index++ )
	{
		const auto& [ render_group_id, render_group_material ] = *( render_group_material_info_list.begin() + render_group_index );

		node_drawable_array_map[ render_group_id ].resize( mesh_instance_count );
		auto& drawable_array = node_drawable_array_map[ render_group_id ];

		for( auto i = 0; i < mesh_instance_count; i++ )
		{
			drawable_array[ i ] = Engine::Drawable( &meshes[ i ], ( render_group_material ? render_group_material : &node_material_array[ i ] ),
													node_material_array[ i ].HasUniform( "uniform_transform_world" ) ? &node_transform_array[ i ] : nullptr );
		}
	}

	/* Apply scene-graph transformations: */

	int mesh_index = 0;
	std::function< void( const std::size_t, const Matrix4x4& ) > ProcessNode = [ & ]( const std::size_t node_index, const Matrix4x4& parent_transform )
	{
		const auto& node = nodes[ node_index ];

		auto transform_so_far = node.transform_local * parent_transform;

		for( auto& child_index : node.children )
			ProcessNode( child_index, transform_so_far );

		if( node.mesh_group )
			for( auto& sub_mesh : node.mesh_group->sub_meshes )
				node_transform_array[ mesh_index++ ].SetFromSRTMatrix( transform_so_far );
	};

	for( auto top_level_node_index : model->TopLevelNodeIndices() )
		ProcessNode( top_level_node_index, Engine::Matrix::Scaling( scale ) * Engine::Math::QuaternionToMatrix( rotation ) * Engine::Matrix::Translation( translation ) );
}

ModelInstance::~ModelInstance()
{
}

void ModelInstance::SetMaterialData( Engine::Shader* const shader,
									 const Engine::Texture* diffuse_texture,
									 const Engine::Texture* specular_texture,
									 const Vector4 texture_scale_and_offset )
{
	int material_index = 0;

	node_material_array.resize( model->MeshInstanceCount() );
	phong_material_data_array.resize( model->MeshInstanceCount() );

	const auto node_count = model->NodeCount();
	const auto& nodes = model->Nodes();

	const auto& model_textures = model->Textures();

	for( auto& node : nodes )
	{
		if( node.mesh_group ) // Only process Nodes with Meshes.
		{
			for( auto& sub_mesh : node.mesh_group->sub_meshes )
			{
				auto& material = node_material_array[ material_index ] = Engine::Material( model->Name() + "_" + sub_mesh.name, shader );

				if( diffuse_texture || sub_mesh.texture_albedo )
				{
					phong_material_data_array[ material_index ] =
					{
						.color_diffuse       = {},
						.has_texture_diffuse = 1,
						.shininess           = 32.0f
					};

					material.SetTexture( "uniform_diffuse_map_slot", diffuse_texture ? diffuse_texture : sub_mesh.texture_albedo );
				}
				else if( sub_mesh.color_albedo )
				{
					phong_material_data_array[ material_index ] =
					{
						.color_diffuse       = *sub_mesh.color_albedo,
						.has_texture_diffuse = 0,
						.shininess           = 32.0f
					};
				}

				if( specular_texture )
					material.SetTexture( "uniform_specular_map_slot", specular_texture );

				material.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );

				material_index++;
			}
		}
	}

	for( auto i = 0; i < phong_material_data_array.size(); i++ )
		node_material_array[ i ].Set( "PhongMaterialData", phong_material_data_array[ i ] );
}