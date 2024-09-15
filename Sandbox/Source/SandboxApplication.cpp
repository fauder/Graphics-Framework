// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/AssetDatabase.hpp"
#include "Engine/Core/ImGuiDrawer.hpp"
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/ImGuiUtility.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Graphics/GLLogger.h"
#include "Engine/Graphics/MeshUtility.hpp"
#include "Engine/Graphics/Primitive/Primitive_Cube.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Random.hpp"

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication()
{
    return new SandboxApplication();
}

SandboxApplication::SandboxApplication()
	:
	Engine::Application(),
	light_source_drawable_array( LIGHT_POINT_COUNT ),
	cube_drawable_array( CUBE_COUNT ),
	phong_shader( "Phong" ),
	basic_color_shader( "Basic Color" ),
	camera_transform( Vector3::One(), Quaternion::LookRotation( Vector3{ 0.0f, -0.5f, 1.0f }.Normalized() ), Vector3{ 0.0f, 10.0f, -20.0f } ),
	light_point_transform_array( LIGHT_POINT_COUNT ),
	cube_transform_array( CUBE_COUNT),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
	camera_is_animated( false ),
	auto_calculate_aspect_ratio( true ),
	auto_calculate_vfov_based_on_90_hfov( true ),
	ui_interaction_enabled( false ),
	show_imgui_demo_window( false )
{
	Initialize();
}

SandboxApplication::~SandboxApplication()	
{
	Shutdown();
}

void SandboxApplication::Initialize()
{
	renderer.SetClearColor( Engine::Color4::Gray() );

	Platform::ChangeTitle( "Sandbox (Graphics Framework)" );

	Engine::ServiceLocator< Engine::GLLogger >::Register( &gl_logger );

	gl_logger.IgnoreID( 131185 );

	//Engine::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox GL Init.", true /* omit if the group is empty */ ) );

/* Textures: */
	Engine::Texture::ImportSettings texture_import_settings;
	container_texture_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Diffuse) Map",	R"(Asset/Texture/container2.png)",			texture_import_settings );
	container_texture_specular_map = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Specular) Map", R"(Asset/Texture/container2_specular.png)", texture_import_settings );
	
	texture_import_settings.wrap_u = GL_REPEAT;
	texture_import_settings.wrap_v = GL_REPEAT;
	checker_pattern = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Checkerboard Pattern (09)", R"(Asset/Texture/kenney_prototype/texture_09.png)", texture_import_settings );

/* Shaders: */
	phong_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	basic_color_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/BasicColor.frag)" );

/* Initial transforms: */
	ground_quad_transform.SetScaling( 25.0f, 0.01f, 125.0f );

	front_wall_quad_transform
		.SetScaling( 25.0f, 25.0f, 0.01f )
		.SetTranslation( Vector3::Forward() * 15.0f );

	for( auto cube_index = 0; cube_index < CUBE_COUNT; cube_index++ )
	{
		Degrees angle( 20.0f * cube_index );
		cube_transform_array[ cube_index ]
			.SetRotation( Quaternion( angle, Vector3{ 1.0f, 0.3f, 0.5f }.Normalized() ) )
			.SetTranslation( CUBE_POSITIONS[ cube_index ] + Vector3::Up() * 5.0f );
	}

/* Vertex/Index Data: */
	cube_mesh = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube::Positions.cbegin(), Engine::Primitive::NonIndexed::Cube::Positions.cend() ),
							  "Cube",
							  std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube::Normals.cbegin(), Engine::Primitive::NonIndexed::Cube::Normals.cend() ),
							  std::vector< Vector2 >( Engine::Primitive::NonIndexed::Cube::UVs.cbegin(), Engine::Primitive::NonIndexed::Cube::UVs.cend() ),
							  { /* No indices. */ } );

	Engine::Model::ImportSettings model_import_settings( GL_STATIC_DRAW );
	test_model     = Engine::AssetDatabase< Engine::Model >::CreateAssetFromFile( "Deccer Cubes",	R"(C:/users/fauder/desktop/SM_Deccer_Cubes_Textured_Complex.gltf)", model_import_settings );

/* Lighting: */
	ResetLightingData();

	renderer.AddDirectionalLight( &light_directional );

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer.AddPointLight( &light_point_array[ index ] );

	renderer.AddSpotLight( &light_spot );

/* Materials: */
	ResetMaterialData();

	ground_quad_material.Set( "PhongMaterialData", ground_quad_surface_data );
	front_wall_quad_material.Set( "PhongMaterialData", front_wall_quad_surface_data );
	for( auto i = 0; i < CUBE_COUNT; i++ )
		cube_material_array[ i ].Set( "PhongMaterialData", cube_surface_data_array[ i ] );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ].Set( "uniform_color", light_point_array[ i ].data.diffuse_and_attenuation_linear.color );
		
	for( auto i = 0; i < test_material_array.size(); i++ )
		test_material_array[ i ].Set( "PhongMaterialData", test_sub_mesh_surface_data_array[ i ] );

/* Drawables & the Renderer: */
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_source_drawable_array[ i ] = Engine::Drawable( &cube_mesh, &light_source_material_array[ i ], &light_point_transform_array[ i ] );
		renderer.AddDrawable( &light_source_drawable_array[ i ] );
	}

	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		cube_drawable_array[ i ] = Engine::Drawable( &cube_mesh, &cube_material_array[ i ], &cube_transform_array[ i ] );
		renderer.AddDrawable( &cube_drawable_array[ i ] );
	}

	auto InitializeModelData = [ & ]( const Engine::Model* model,
									  std::vector< Engine::Transform >& sub_mesh_transform_array, std::vector< Engine::Drawable >& drawable_array, std::vector< Engine::Material >& material_array,
									  const Vector3 scale, const Quaternion& rotation, const Vector3& translation )
	{
		if( model )
		{
			const auto mesh_instance_count = model->MeshInstanceCount();
			const auto& meshes             = model->Meshes();
			const auto& nodes              = model->Nodes();

			sub_mesh_transform_array.resize( mesh_instance_count );
			drawable_array.resize( mesh_instance_count );

			for( auto i = 0; i < mesh_instance_count; i++ )
			{
				drawable_array[ i ] = Engine::Drawable( &meshes[ i ], &material_array[ i ], &sub_mesh_transform_array[ i ] );
				renderer.AddDrawable( &drawable_array[ i ] );
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
						sub_mesh_transform_array[ mesh_index++ ].SetFromSRTMatrix( transform_so_far );
			};

			for( auto top_level_node_index : model->TopLevelNodeIndices() )
				ProcessNode( top_level_node_index, Engine::Matrix::Scaling( scale ) * Engine::Math::QuaternionToMatrix( rotation ) * Engine::Matrix::Translation( translation ) );
		}
	};

	InitializeModelData( test_model, test_sub_mesh_transform_array, test_drawable_array, test_material_array,
						 Vector3::One(), Quaternion(), Vector3::Left() * 2 + Vector3::Up() * 8.0f );

	ground_quad_drawable = Engine::Drawable( &cube_mesh, &ground_quad_material, &ground_quad_transform );
	renderer.AddDrawable( &ground_quad_drawable );

	front_wall_quad_drawable = Engine::Drawable( &cube_mesh, &front_wall_quad_material, &front_wall_quad_transform );
	renderer.AddDrawable( &front_wall_quad_drawable );	

/* Other: */
	// Also add a spot-light to the "led" node/mesh of the spotlight model:
	if( auto led_node_iterator = std::find_if( test_model->Nodes().cbegin(), test_model->Nodes().cend(), []( const Engine::Model::Node& node ) { return node.name.find( "led" ) != std::string::npos; } );
		led_node_iterator != test_model->Nodes().cend() )
	{
		const std::size_t index = std::distance( test_model->Nodes().cbegin(), led_node_iterator );
		auto& led_sub_mesh_transform = test_sub_mesh_transform_array[ index ];

		light_spot_transform = led_sub_mesh_transform;
		light_spot_transform.MultiplyRotation_X( 90.0_deg );
		renderer.AddSpotLight( &light_spot );
	}

	renderer.EnableDepthTest();

	Platform::MaximizeWindow();
}

void SandboxApplication::Shutdown()
{
	/* Insert application-specific shutdown code here. */
}

//void SandboxApplication::Run()
//{
//
//}

void SandboxApplication::Update()
{
	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Update", true /* omit if the group is empty */ ) );

	current_time_as_angle = Radians( time_current );

	/* Light sources' transform: */
	constexpr Radians angle_increment( Engine::Constants< Radians >::Two_Pi() / LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		/* Light source transform: */
		Vector3 point_light_position_world_space;

		if( light_point_array_is_animated )
		{
			const auto point_light_rotation = Engine::Matrix::RotationAroundY( current_time_as_angle * 0.33f + angle_increment * ( float )i );

			point_light_position_world_space = ( ( Vector4::Forward() * light_point_orbit_radius ) *
												 ( point_light_rotation * Engine::Matrix::Translation( CUBES_ORIGIN ) ) ).XYZ();
			point_light_position_world_space.SetY( 2.0f );

			light_point_transform_array[ i ].SetRotation( Engine::Math::MatrixToQuaternion( point_light_rotation ) );
		}

		light_point_transform_array[ i ].SetTranslation( point_light_position_world_space );
	}

	/* Camera transform: */
	if( camera_is_animated )
	{
		/* Orbit motion: */

		Engine::Math::Vector< Radians, 3 > old_euler_angles;
		Engine::Math::QuaternionToEuler( camera_transform.GetRotation(), old_euler_angles );
		// Don't modify X & Z euler angles; Allow the user to modify them.
		camera_transform.SetRotation( Engine::Math::EulerToQuaternion( -current_time_as_angle * 0.33f, old_euler_angles.X(), old_euler_angles.Z() ) );

		auto new_pos = CUBES_ORIGIN + -camera_transform.Forward() * 30.0f;
		new_pos.SetY( camera_transform.GetTranslation().Y() ); // Don't modify Y position; Allow the user to modify it.
		camera_transform.SetTranslation( new_pos );
	}
	else
	{
		if( !ui_interaction_enabled )
		{
			// Control via mouse:
			const auto [ mouse_x_delta_pos, mouse_y_delta_pos ] = Platform::GetMouseCursorDeltas();
			camera_controller
				.OffsetHeading( Radians( +mouse_x_delta_pos ) )
				.OffsetPitch( Radians( +mouse_y_delta_pos ), -60.0_deg, +60.0_deg );
		}
	}

	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_W ) )
		camera_transform.OffsetTranslation( camera_transform.Forward() * +camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_S ) )
		camera_transform.OffsetTranslation( camera_transform.Forward() * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_A ) )
		camera_transform.OffsetTranslation( camera_transform.Right()   * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_D ) )
		camera_transform.OffsetTranslation( camera_transform.Right()   * +camera_move_speed * time_delta );

	renderer.Update( camera );
}

void SandboxApplication::Render()
{
	UpdateViewMatrix();

	Engine::Application::Render();

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Render", true /* omit if the group is empty */ ) );

	renderer.Render( camera );
}

void SandboxApplication::RenderImGui()
{
	{
		auto log_group( gl_logger.TemporaryLogGroup( "Application ImGui", true /* omit if the group is empty */ ) );

		Application::RenderImGui();
	}

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox ImGui", true /* omit if the group is empty */ ) );

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	Engine::ImGuiDrawer::Draw( phong_shader );
	Engine::ImGuiDrawer::Draw( basic_color_shader );

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		Engine::ImGuiDrawer::Draw( light_source_material_array[ i ] );
	Engine::ImGuiDrawer::Draw( ground_quad_material );
	Engine::ImGuiDrawer::Draw( front_wall_quad_material );
	for( auto& cube_material : cube_material_array )
		Engine::ImGuiDrawer::Draw( cube_material );
	for( auto& test_material : test_material_array )
		Engine::ImGuiDrawer::Draw( test_material );

	if( ImGui::Begin( "Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
			ResetLightingData();
		
		ImGui::NewLine();

		if( ImGui::BeginTabBar( "Lights Tab Bar", ImGuiTabBarFlags_DrawSelectedOverline | ImGuiTabBarFlags_NoTabListScrollingButtons ) )
		{
			if( ImGui::BeginTabItem( "Directional Light" ) )
			{
				Engine::ImGuiDrawer::Draw( light_directional, "Directional Light" );
				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Point Lights" ) )
			{
				Engine::ImGuiUtility::BeginGroupPanel( "Options" );
				if( ImGui::Checkbox( "Disable All", &light_point_array_disable ) )
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
						light_point_array[ i ].is_enabled = !light_point_array_disable;
				ImGui::Checkbox( "Animate (Orbit) Point Lights", &light_point_array_is_animated );
				if( light_point_array_is_animated )
					ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 15.0f );
				Engine::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str(), light_point_array_is_animated /* hide position. */ ) )
							light_source_material_array[ i ].Set( "uniform_color", light_point_array[ i ].data.diffuse_and_attenuation_linear.color );
					}

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Spot Lights" ) )
			{
				if( ImGui::TreeNodeEx( "Spot Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					Engine::ImGuiDrawer::Draw( light_spot, "Spot Light" );

					ImGui::TreePop();
				}
			
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		for( auto i = 0; i < CUBE_COUNT; i++ )
		{
			auto& surface_data = cube_surface_data_array[ i ];
			Engine::ImGuiDrawer::Draw( surface_data, ( "Cube #" + std::to_string( i + 1 ) + " Surface Properties" ).c_str() );
		}

		Engine::ImGuiDrawer::Draw( ground_quad_surface_data,	 "Ground Quad Surface Properties" );
		Engine::ImGuiDrawer::Draw( front_wall_quad_surface_data, "Front Wall Surface Properties"  );
	}

	ImGui::End();

	if( ImGui::Begin( "Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			camera_transform.SetTranslation( 0.0f, 10.0f, -20.0f );
			camera_transform.LookAt( Vector3{ 0.0f, -0.5f, 1.0f }.Normalized() );
			
			camera_is_animated = false;
		}

		ImGui::Checkbox( "Animate (Rotate) Camera", &camera_is_animated );
		Engine::ImGuiDrawer::Draw( camera_transform, Engine::Transform::Mask::NoScale, "Main Camera" );
	}

	ImGui::End();

	if( ImGui::Begin( "Projection", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			auto_calculate_aspect_ratio          = true;
			auto_calculate_vfov_based_on_90_hfov = true;

			camera.SetAspectRatio( Platform::GetAspectRatio() ); // If we don't do this, CalculateVerticalFieldOfView() below will work with possibly the old aspect ratio and produce incorrect results.
			camera = Engine::Camera( &camera_transform, camera.GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );
		}

		Engine::ImGuiDrawer::Draw( camera, "Main Camera" );

		float v_fov_radians = ( float )camera.GetVerticalFieldOfView();

		bool modified = false;

		modified |= ImGui::Checkbox( "Auto-calculate Aspect Ratio",										&auto_calculate_aspect_ratio );
		modified |= ImGui::Checkbox( "Auto-calculate Vertical FoV to match 90 degrees Horizontal FoV",	&auto_calculate_vfov_based_on_90_hfov );

		if( modified )
		{
			if( auto_calculate_aspect_ratio )
				camera.SetAspectRatio( Platform::GetAspectRatio() );
			
			if( auto_calculate_vfov_based_on_90_hfov )
				camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );
			else
				camera.SetVerticalFieldOfView( Radians( v_fov_radians ) );
		}
	}

	ImGui::End();

	renderer.RenderImGui();
}

void SandboxApplication::UpdateViewMatrix()
{
	view_transformation = camera.GetViewMatrix();
}

void SandboxApplication::ResetLightingData()
{
	light_directional = 
	{
		.is_enabled = true,
		.data =
		{
			.ambient               = Engine::Color3{  0.05f,  0.05f,  0.05f },
			.diffuse               = Engine::Color3{  0.4f,   0.4f,   0.4f  },
			.specular              = Engine::Color3{  0.5f,   0.5f,   0.5f  },
		},
		.transform = &light_directional_transform
	};

	light_point_array_disable      = false;
	light_point_array_is_animated  = true;
	light_point_orbit_radius       = 13.25f;

	light_spot_array_disable = false;

	light_point_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_point_array[ i ] =
		{
			.is_enabled = true,
			.data =
			{
				.ambient_and_attenuation_constant = {.color = {  0.05f,  0.05f,  0.05f },							.scalar = 0.075f	},
				.diffuse_and_attenuation_linear   = {.color = Engine::Math::Random::Generate< Engine::Color3 >(),	.scalar = 0.0035f	},
				.specular_attenuation_quadratic   = {.color = {  1.0f,   1.0f,   1.0f  },							.scalar = 0.09f },
			},
			.transform = &light_point_transform_array[ i ]
		};
	}

	light_spot =
	{
		.is_enabled = true,
		.data =
		{
			.ambient  = Engine::Color3{  0.05f,  0.05f,  0.03f },
			.diffuse  = Engine::Color3{  0.4f,   0.4f,   0.27f  },
			.specular = Engine::Color3{  0.5f,   0.5f,   0.33f  },

		/* End of GLSL equivalence. */
			.cutoff_angle_inner     = 12.5_deg,
			.cutoff_angle_outer     = 17.5_deg
		},
		.transform = &light_spot_transform
	};
}

void SandboxApplication::ResetMaterialData()
{
	light_source_material_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ] = Engine::Material( "Light Source #" + std::to_string( i + 1 ), &basic_color_shader );
	
	cube_material_array.resize( CUBE_COUNT );
	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		cube_material_array[ i ] = Engine::Material( "Cube #" + std::to_string( i + 1 ), &phong_shader );
		cube_material_array[ i ].SetTexture( "uniform_diffuse_map_slot", container_texture_diffuse_map );
		cube_material_array[ i ].SetTexture( "uniform_specular_map_slot", container_texture_specular_map );
		cube_material_array[ i ].Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );
	}

	ground_quad_material = Engine::Material( "Ground", &phong_shader );
	ground_quad_material.SetTexture( "uniform_diffuse_map_slot", checker_pattern );
	ground_quad_material.SetTexture( "uniform_specular_map_slot", checker_pattern );
	const auto& ground_quad_scale( ground_quad_transform.GetScaling() );
	Vector4 ground_texture_scale_and_offset( ground_quad_scale.X(), ground_quad_scale.Z() /* Offset is 0 so no need to set it explicitly. */ );
	ground_quad_material.Set( "uniform_texture_scale_and_offset", ground_texture_scale_and_offset );

	front_wall_quad_material = Engine::Material( "Front Wall", &phong_shader );
	front_wall_quad_material.SetTexture( "uniform_diffuse_map_slot", checker_pattern );
	front_wall_quad_material.SetTexture( "uniform_specular_map_slot", checker_pattern );
	const auto& front_wall_quad_scale( front_wall_quad_transform.GetScaling() );
	Vector4 front_wall_texture_scale_and_offset( front_wall_quad_scale /* Offset is 0 so no need to set it explicitly. */ );
	front_wall_quad_material.Set( "uniform_texture_scale_and_offset", front_wall_texture_scale_and_offset );

	ground_quad_surface_data = front_wall_quad_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	cube_surface_data_array = std::vector< Engine::MaterialData::PhongMaterialData >( CUBE_COUNT, ground_quad_surface_data );

	auto ResetModelMaterialData = []( const Engine::Model* model, const std::string& model_name,
									  std::vector< Engine::Material >& material_array, std::vector< Engine::MaterialData::PhongMaterialData >& surface_data_array, Engine::Shader* shader,
									  Engine::Texture* diffuse_texture, Engine::Texture* specular_texture,
									  const Vector4& texture_scale_and_offset )
	{
		if( model )
		{
			int material_index = 0;

			material_array.resize( model->MeshInstanceCount() );
			surface_data_array.resize( model->MeshInstanceCount() );

			const auto node_count = model->NodeCount();
			const auto& nodes     = model->Nodes();

			const auto& model_textures = model->Textures();

			for( auto& node : nodes )
			{
				if( node.mesh_group ) // Only process Nodes having a Mesh.
				{
					for( auto& sub_mesh : node.mesh_group->sub_meshes )
					{
						auto& material = material_array[ material_index ] = Engine::Material( model_name + "_" + sub_mesh.name, shader);

						if( diffuse_texture || sub_mesh.texture_albedo )
						{
							surface_data_array[ material_index ] = 
							{
								.color_diffuse       = {},
								.has_texture_diffuse = 1,
								.shininess           = 32.0f
							};

							material.SetTexture( "uniform_diffuse_map_slot", diffuse_texture ? diffuse_texture : sub_mesh.texture_albedo );
						}
						else if( sub_mesh.color_albedo )
						{
							surface_data_array[ material_index ] =
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
		}
	};

	const Vector4 identity_texture_scale_and_offset( 1.0f, 1.0f, 0.0f, 0.0f );

	ResetModelMaterialData( test_model, "Test (minigun_m-134)",
							test_material_array, test_sub_mesh_surface_data_array, &phong_shader, nullptr /* Use Albedo colors in-model. */, checker_pattern,
							identity_texture_scale_and_offset );
}

SandboxApplication::Radians SandboxApplication::CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const
{
	return 2.0f * Engine::Math::Atan2( Engine::Math::Tan( horizontal_field_of_view / 2.0f ), camera.GetAspectRatio() );
}

void SandboxApplication::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
{
	switch( key_code )
	{
		case Platform::KeyCode::KEY_ESCAPE:
			if( key_action == Platform::KeyAction::PRESS )
				Platform::SetShouldClose( true );
			break;
		/* Use the key below ESC to toggle between game & menu/UI. */
		case Platform::KeyCode::KEY_GRAVE_ACCENT:
			if( key_action == Platform::KeyAction::PRESS )
				ui_interaction_enabled = !ui_interaction_enabled;
			break;
		case Platform::KeyCode::KEY_W:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Forward() * +camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_S:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Forward() * -camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_A:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Right() * -camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_D:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Right() * +camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_U:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot.data.cutoff_angle_inner = Engine::Math::Min( light_spot.data.cutoff_angle_inner + 0.33_deg, light_spot.data.cutoff_angle_outer );
			break;
		case Platform::KeyCode::KEY_Y:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot.data.cutoff_angle_inner = Engine::Math::Max( light_spot.data.cutoff_angle_inner - 0.33_deg, 0_deg );
			break;
		case Platform::KeyCode::KEY_I:
			if( key_action == Platform::KeyAction::PRESS )
				show_imgui = !show_imgui;
			break;
		case Platform::KeyCode::KEY_O:
			if( key_action == Platform::KeyAction::PRESS )
				show_imgui_demo_window = !show_imgui_demo_window;
			break;
		default:
			break;
	}
}

void SandboxApplication::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
{
	// Re-calculate the aspect ratio:
	if( auto_calculate_aspect_ratio )
	{
		camera.SetAspectRatio( float( width_new_pixels ) / height_new_pixels );
		if( auto_calculate_vfov_based_on_90_hfov )
			camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );

		renderer.OnProjectionParametersChange( camera );
	}
}