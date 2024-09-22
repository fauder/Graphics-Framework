#define IMGUI_DEFINE_MATH_OPERATORS

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
#include "Engine/Graphics/Primitive/Primitive_Quad.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Random.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.
#include <fstream>

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
	cube_drawable_outline_array( CUBE_COUNT ),
	phong_shader( "Phong" ),
	basic_color_shader( "Basic Color" ),
	outline_shader( "Outline" ),
	light_point_transform_array( LIGHT_POINT_COUNT ),
	cube_transform_array( CUBE_COUNT),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
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
	renderer.SetClearTargets( Engine::Renderer::ClearTarget::All );

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
	outline_shader.FromFile( R"(Asset/Shader/Outline.vert)", R"(Asset/Shader/BasicColor.frag)" );

/* Initial transforms: */
	ground_quad_transform
		.SetScaling( 25.0f, 75.0f, 1.0f )
		.SetRotation( 0.0_deg, 90.0_deg, 0.0_deg );

	front_wall_quad_transform
		.SetScaling( 25.0f, 25.0f, 1.0f )
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

	quad_mesh = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Quad::Positions.cbegin(), Engine::Primitive::NonIndexed::Quad::Positions.cend() ),
							  "Quad",
							  std::vector< Vector3 >( Engine::Primitive::NonIndexed::Quad::Normals.cbegin(), Engine::Primitive::NonIndexed::Quad::Normals.cend() ),
							  std::vector< Vector2 >( Engine::Primitive::NonIndexed::Quad::UVs.cbegin(), Engine::Primitive::NonIndexed::Quad::UVs.cend() ),
							  { /* No indices. */ } );

/* Lighting: */
	ResetLightingData();

	renderer.AddDirectionalLight( &light_directional );

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer.AddPointLight( &light_point_array[ index ] );

	renderer.AddSpotLight( &light_spot );

/* Materials: */
	ResetMaterialData();

/* Renderer (Drawables, RenderStates etc.): */
	renderer.SetRenderGroupName( 0, "Default" );
	auto& render_state_regular_meshes = renderer.GetRenderState( 0 ); // Keep default settings.
	
	{
		/* This pass draws the mesh semi-regularly; It also marks the stencil buffer with 1s everywhere the mesh is drawn at. */
		renderer.SetRenderGroupName( 1, "Outlined Meshes" );
		auto& render_state_outline_meshes = renderer.GetRenderState( 1 );

		/* Omitted settings are left to defaults. */

		render_state_outline_meshes.stencil_test_enable                           = true;
		render_state_outline_meshes.stencil_write_mask                            = 0xFF;
		render_state_outline_meshes.stencil_comparison_function                   = Engine::Renderer::ComparisonFunction::Always;
		render_state_outline_meshes.stencil_ref                                   = 0x01;
		render_state_outline_meshes.stencil_test_response_stencil_fail            = Engine::Renderer::StencilTestResponse::Keep;
		render_state_outline_meshes.stencil_test_response_stencil_pass_depth_fail = Engine::Renderer::StencilTestResponse::Keep;
		render_state_outline_meshes.stencil_test_response_both_pass               = Engine::Renderer::StencilTestResponse::Replace;
	}

	{
		/* This pass draws the outlines only; It does this by rendering the mesh at everywhere the stencil buffer is NOT 1. */
		renderer.SetRenderGroupName( 2, "Outlines" );
		auto& render_state_outlines = renderer.GetRenderState( 2 );

		/* Omitted settings are left to defaults. */

		render_state_outlines.depth_test_enable = false;

		render_state_outlines.stencil_test_enable         = true;
		render_state_outlines.stencil_write_mask          = 0x00; // Disable writes; This pass only needs to READ the stencil buffer, to figure out where NOT to render.
		render_state_outlines.stencil_comparison_function = Engine::Renderer::ComparisonFunction::NotEqual; // Render everywhere that's not the actual mesh, i.e., the border.
		render_state_outlines.stencil_ref                 = 0x01;
	}

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_source_drawable_array[ i ] = Engine::Drawable( &cube_mesh, &light_source_material_array[ i ], &light_point_transform_array[ i ] );
		renderer.AddDrawable( &light_source_drawable_array[ i ], Engine::Renderer::RenderGroupID( 0 ) );
	}

	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		cube_drawable_array[ i ] = Engine::Drawable( &cube_mesh, &cube_material_array[ i ], &cube_transform_array[ i ] );
		renderer.AddDrawable( &cube_drawable_array[ i ], Engine::Renderer::RenderGroupID( 1 ) );

		cube_drawable_outline_array[ i ] = Engine::Drawable( &cube_mesh, &outline_material, &cube_transform_array[ i ] );
		renderer.AddDrawable( &cube_drawable_outline_array[ i ], Engine::Renderer::RenderGroupID( 2 ) );
	}

	ground_quad_drawable = Engine::Drawable( &quad_mesh, &ground_quad_material, &ground_quad_transform );
	renderer.AddDrawable( &ground_quad_drawable, Engine::Renderer::RenderGroupID( 0 ) );

	front_wall_quad_drawable = Engine::Drawable( &quad_mesh, &front_wall_quad_material, &front_wall_quad_transform );
	renderer.AddDrawable( &front_wall_quad_drawable, Engine::Renderer::RenderGroupID( 0 ) );

/* Models: */
	if( auto config_file = std::ifstream( "config.ini" ) )
	{
		std::string ignore;
		config_file >> ignore;
		if( ignore == "test_model_path" )
		{
			config_file >> ignore /* '=' */ >> test_model_file_path;

			if( !test_model_file_path.empty() )
				ReloadModel( test_model_file_path );
		}
	}

/* Other: */
	ResetCamera();

	Platform::MaximizeWindow();
}

void SandboxApplication::Shutdown()
{
	if( auto config_file = std::ofstream( "config.ini" ) )
	{
		config_file << "test_model_path = " << test_model_file_path;
	}
}

//void SandboxApplication::Run()
//{
//
//}

void SandboxApplication::Update()
{
	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Update", true /* omit if the group is empty */ ) );

	current_time_as_angle = Radians( time_current );
	const Radians current_time_mod_two_pi( std::fmod( time_current, Engine::Constants< float >::Two_Pi() ) );

	/* Light sources' transform: */
	constexpr Radians angle_increment( Engine::Constants< Radians >::Two_Pi() / LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		/* Light source transform: */
		Vector3 point_light_position_world_space;

		if( light_point_array_is_animated )
		{
			const auto old_rotation( light_point_transform_array[ i ].GetRotation() );
			Radians old_heading, old_pitch, bank;
			Engine::Math::QuaternionToEuler( old_rotation, old_heading, old_pitch, bank );

			Radians new_angle( current_time_mod_two_pi + ( float )angle_increment * ( float )i );
			const Radians new_angle_mod_two_pi( std::fmod( new_angle.Value(), Engine::Constants< float >::Two_Pi() ) );

			const bool heading_instead_of_pitching = new_angle_mod_two_pi <= Engine::Constants< Radians >::Pi();

			const auto point_light_rotation( Engine::Math::EulerToMatrix( float(  heading_instead_of_pitching ) * new_angle_mod_two_pi,
																		  float( !heading_instead_of_pitching ) * ( new_angle_mod_two_pi - 3.0f * Engine::Constants< Radians >::Pi_Over_Two() ), bank ) );

			point_light_position_world_space = ( ( ( heading_instead_of_pitching ? Vector4::Forward() : Vector4::Up() ) * light_point_orbit_radius ) *
												 ( point_light_rotation * Engine::Matrix::Translation( CUBES_ORIGIN ) ) ).XYZ();
			point_light_position_world_space.SetY( point_light_position_world_space.Y() + 2.0f );

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

	const auto& style = ImGui::GetStyle();

	if( ImGui::Begin( ICON_FA_CUBES " Models", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		bool load_button_is_clicked = false;

		if( not test_model_file_path.empty() )
		{
			static char buffer[ 260 ];
			strncpy_s( buffer, test_model_file_path.c_str(), test_model_file_path.size());
			ImGui::BeginDisabled();
			ImGui::InputText( "Loaded Model Path", buffer, ( int )test_model_file_path.size(), ImGuiInputTextFlags_ReadOnly);
			ImGui::EndDisabled();

			if( ImGui::Button( ICON_FA_FOLDER_OPEN " Reload" ) )
			{
				if( auto maybe_file_name = Platform::BrowseFileName( {	"glTF (*.gltf;*.glb)",		"*.gltf;*.glb",	
																		"Standard glTF (*.gltf)",	"*.gltf",
																		"Binary glTF (*.glb)",		"*.glb" },
																		"Choose a Model to Load" );
					maybe_file_name.has_value() && *maybe_file_name != test_model_file_path )
				{
					ReloadModel( *maybe_file_name );
				}
			}
			ImGui::SameLine();
			if( ImGui::Button( ICON_FA_XMARK " Unload" ) )
				UnloadModel();
		}
		else
		{
			const auto button_size( ImGui::CalcTextSize( ICON_FA_CUBES " Models   " ) + style.ItemInnerSpacing );
			if( ImGui::Button( ICON_FA_FOLDER_OPEN " Load", button_size ) )
			{
				if( auto maybe_file_name = Platform::BrowseFileName( {	"glTF (*.gltf;*.glb)",		"*.gltf;*.glb",	
																		"Standard glTF (*.gltf)",	"*.gltf",
																		"Binary glTF (*.glb)",		"*.glb" },
																	 "Choose a Model to Load" );
					maybe_file_name.has_value() && *maybe_file_name != test_model_file_path )
				{
					ReloadModel( *maybe_file_name );
				}
			}
		}
	}

	ImGui::End();

	/* Shaders: */
	{
		Engine::ImGuiDrawer::Draw( phong_shader );
		Engine::ImGuiDrawer::Draw( basic_color_shader );
		Engine::ImGuiDrawer::Draw( outline_shader );
	}

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		Engine::ImGuiDrawer::Draw( light_source_material_array[ i ] );
	Engine::ImGuiDrawer::Draw( ground_quad_material );
	Engine::ImGuiDrawer::Draw( front_wall_quad_material );
	for( auto& cube_material : cube_material_array )
		Engine::ImGuiDrawer::Draw( cube_material );
	for( auto& test_material : test_model_instance.Materials() )
		Engine::ImGuiDrawer::Draw( const_cast< Engine::Material& >( test_material ) );
	Engine::ImGuiDrawer::Draw( outline_material );

	if( ImGui::Begin( ICON_FA_LIGHTBULB " Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Checkbox( "Enabled##AllLights", &light_is_enabled ) )
		{
			light_directional.is_enabled = light_is_enabled;

			for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
				light_source_drawable_array[ i ].ToggleOnOrOff( light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable );

			light_spot.is_enabled = light_is_enabled;
		}

		ImGui::SameLine( 0.0f, 20.0f );

		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Lights" ) )
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
						light_source_drawable_array[ i ].ToggleOnOrOff( light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable );
				ImGui::Checkbox( "Animate (Orbit) Point Lights", &light_point_array_is_animated );
				if( light_point_array_is_animated )
					ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 15.0f );
				Engine::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str(), light_point_array_is_animated /* hide position. */ ) )
						{
							light_source_material_array[ i ].Set( "uniform_color", light_point_array[ i ].data.diffuse_and_attenuation_linear.color );
							if( was_enabled != light_point_array[ i ].is_enabled )
								light_source_drawable_array[ i ].ToggleOnOrOff();
						}
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
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_VIDEO " Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Camera" ) )
			ResetCamera();

		ImGui::SameLine(); if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset to 2nd View" ) )
			ResetCamera_2ndView();

		ImGui::Checkbox( "Animate (Rotate) Camera", &camera_is_animated );
		Engine::ImGuiDrawer::Draw( camera_transform, Engine::Transform::Mask::NoScale, "Main Camera" );

		if( ImGui::CollapsingHeader( "Projection" ) )
		{
			Engine::ImGuiUtility::BeginGroupPanel();
			{
				if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset" ) )
				{
					auto_calculate_aspect_ratio = true;
					auto_calculate_vfov_based_on_90_hfov = true;

					camera.SetAspectRatio( Platform::GetAspectRatio() ); // If we don't do this, CalculateVerticalFieldOfView() below will work with possibly the old aspect ratio and produce incorrect results.
					camera = Engine::Camera( &camera_transform, camera.GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );
				}

				bool modified = false;

				modified |= Engine::ImGuiDrawer::Draw( camera, "Main Camera" );

				float v_fov_radians = ( float )camera.GetVerticalFieldOfView();

				modified |= ImGui::Checkbox( "Auto-calculate Aspect Ratio", &auto_calculate_aspect_ratio );
				modified |= ImGui::Checkbox( "Auto-calculate Vertical FoV to match 90 degrees Horizontal FoV", &auto_calculate_vfov_based_on_90_hfov );

				if( modified )
				{
					if( auto_calculate_aspect_ratio )
						camera.SetAspectRatio( Platform::GetAspectRatio() );

					if( auto_calculate_vfov_based_on_90_hfov )
						camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) );
					else
						camera.SetVerticalFieldOfView( Radians( v_fov_radians ) );

					renderer.OnProjectionParametersChange( camera );
				}
			}
			Engine::ImGuiUtility::EndGroupPanel();
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

	light_is_enabled = true;

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
	}

	ground_quad_material = Engine::Material( "Ground", &phong_shader );
	ground_quad_material.SetTexture( "uniform_diffuse_map_slot", checker_pattern );
	ground_quad_material.SetTexture( "uniform_specular_map_slot", checker_pattern );
	const auto& ground_quad_scale( ground_quad_transform.GetScaling() );
	Vector4 ground_texture_scale_and_offset( ground_quad_scale.X(), ground_quad_scale.Y() /* Offset is 0 so no need to set it explicitly. */ );
	ground_quad_material.Set( "uniform_texture_scale_and_offset", ground_texture_scale_and_offset );

	front_wall_quad_material = Engine::Material( "Front Wall", &phong_shader );
	front_wall_quad_material.SetTexture( "uniform_diffuse_map_slot", checker_pattern );
	front_wall_quad_material.SetTexture( "uniform_specular_map_slot", checker_pattern );
	const auto& front_wall_quad_scale( front_wall_quad_transform.GetScaling() );
	Vector4 front_wall_texture_scale_and_offset( front_wall_quad_scale /* Offset is 0 so no need to set it explicitly. */ );
	front_wall_quad_material.Set( "uniform_texture_scale_and_offset", front_wall_texture_scale_and_offset );

	outline_material = Engine::Material( "Outline", &outline_shader );

	ground_quad_surface_data = front_wall_quad_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	cube_surface_data_array = std::vector< Engine::MaterialData::PhongMaterialData >( CUBE_COUNT, ground_quad_surface_data );

	ground_quad_material.Set( "PhongMaterialData", ground_quad_surface_data );
	front_wall_quad_material.Set( "PhongMaterialData", front_wall_quad_surface_data );
	for( auto i = 0; i < CUBE_COUNT; i++ )
		cube_material_array[ i ].Set( "PhongMaterialData", cube_surface_data_array[ i ] );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ].Set( "uniform_color", light_point_array[ i ].data.diffuse_and_attenuation_linear.color );

	outline_material.Set( "uniform_color", Engine::Color4::Orange() );
	outline_material.Set( "uniform_outline_thickness", 0.1f );
}

void SandboxApplication::ResetCamera()
{
	camera_transform.SetTranslation( 0.0f, 10.0f, -20.0f );
	camera_transform.LookAt( Vector3::Forward() );

	camera_is_animated = false;
}

void SandboxApplication::ResetCamera_2ndView()
{
	camera_transform.SetTranslation( 12.0f, 10.0f, -14.0f );
	camera_transform.SetRotation( -35_deg, 0_deg, 0_deg );

	camera_is_animated = false;
}

SandboxApplication::Radians SandboxApplication::CalculateVerticalFieldOfView( const Radians horizontal_field_of_view ) const
{
	return 2.0f * Engine::Math::Atan2( Engine::Math::Tan( horizontal_field_of_view / 2.0f ), camera.GetAspectRatio() );
}

void SandboxApplication::ReloadModel( const std::string& file_path )
{
	Engine::Model::ImportSettings model_import_settings( GL_STATIC_DRAW );
	auto new_model = Engine::AssetDatabase< Engine::Model >::CreateAssetFromFile( "Test Model",
																				  file_path,
																				  model_import_settings );

	if( new_model )
	{
		test_model_file_path = file_path;

		for( auto& [ render_group_id, drawables_array ] : test_model_instance.DrawablesMap() )
			for( auto& drawable_to_remove : drawables_array )
				renderer.RemoveDrawable( &drawable_to_remove );

		test_model_instance = ModelInstance( new_model,
											 &phong_shader,
											 Vector3::One(),
											 Quaternion(),
											 Vector3::Up() * 8.0f,
											 /* Diffuse texture: */ nullptr /* -> Use Albedo colors in-model. */,
											 /* Specular texture: */ checker_pattern,
											 Vector4{ 1.0f, 1.0f, 0.0f, 0.0f },
											 {
												{ Engine::Renderer::RenderGroupID( 1 ), nullptr },
												{ Engine::Renderer::RenderGroupID( 2 ), &outline_material }
											 } );

		for( const auto& [ render_group_id, drawables_array ] : test_model_instance.DrawablesMap() )
			for( auto& drawable_to_add : test_model_instance.Drawables( render_group_id ) )
				renderer.AddDrawable( &drawable_to_add, render_group_id );

		ResetCamera();
	}
}

void SandboxApplication::UnloadModel()
{
	test_model_file_path = "";

	for( auto& [ id, drawables_array ] : test_model_instance.DrawablesMap() )
		for( auto& drawable_to_remove : drawables_array )
			renderer.RemoveDrawable( &drawable_to_remove );

	test_model_instance = {};
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