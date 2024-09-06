// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/AssetDatabase.hpp"
#include "Engine/Core/ImGuiDrawer.hpp"
#include "Engine/Core/ImGuiSetup.h"
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
	light_source_shader( "Basic Color" ),
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
	Platform::ChangeTitle( "Sandbox (Graphics Framework)" );

	Engine::ServiceLocator< Engine::GLLogger >::Register( &gl_logger );

	gl_logger.IgnoreID( 131185 );

	//Engine::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox GL Init.", true /* omit if the group is empty */ ) );

/* Textures: */
	Engine::Texture::ImportSettings import_settings( GL_RGBA );
	container_texture_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Diffuse) Map",	R"(Asset/Texture/container2.png)",			import_settings );
	container_texture_specular_map = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Specular) Map", R"(Asset/Texture/container2_specular.png)", import_settings );
	import_settings.format = GL_RGB;
	import_settings.wrap_u = GL_REPEAT;
	import_settings.wrap_v = GL_REPEAT;
	checker_pattern = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Checkerboard Pattern (09)", R"(Asset/Texture/kenney_prototype/texture_09.png)", import_settings );

/* Shaders: */
	phong_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	light_source_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/BasicColor.frag)" );

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

/* Lighting: */
	ResetLightingData();

/* Materials: */
	ResetMaterialData();
	ground_quad_material.Set( "PhongMaterialData", ground_quad_surface_data );
	front_wall_quad_material.Set( "PhongMaterialData", front_wall_quad_surface_data );
	for( auto i = 0; i < CUBE_COUNT; i++ )
		cube_material_array[ i ].Set( "PhongMaterialData", cube_surface_data_array[ i ] );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ].Set( "uniform_color", light_point_array[ i ].data.diffuse_and_attenuation_linear.color );
	
/* Vertex/Index Data: */
	cube_mesh = Engine::Mesh( std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube::Positions.cbegin(), Engine::Primitive::NonIndexed::Cube::Positions.cend() ),
							  "Cube",
							  std::vector< Vector3 >( Engine::Primitive::NonIndexed::Cube::Normals.cbegin(), Engine::Primitive::NonIndexed::Cube::Normals.cend() ),
							  std::vector< Vector2 >( Engine::Primitive::NonIndexed::Cube::UVs.cbegin(), Engine::Primitive::NonIndexed::Cube::UVs.cend() ),
							  { /* No indices. */ } );

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

	ground_quad_drawable = Engine::Drawable( &cube_mesh, &ground_quad_material, &ground_quad_transform );
	renderer.AddDrawable( &ground_quad_drawable );

	front_wall_quad_drawable = Engine::Drawable( &cube_mesh, &front_wall_quad_material, &front_wall_quad_transform );
	renderer.AddDrawable( &front_wall_quad_drawable );

/* Lights: */
	renderer.AddDirectionalLight( &light_directional );

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer.AddPointLight( &light_point_array[ index ] );

	renderer.AddSpotLight( &light_spot );

/* Other: */
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

		if( light_point_is_animated )
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
					.OffsetPitch( Radians( +mouse_y_delta_pos ), -Engine::Constants< Radians >::Pi_Over_Six(), +Engine::Constants< Radians >::Pi_Over_Six() );
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
	// TODO: Make lights togglable.

	{
		auto log_group( gl_logger.TemporaryLogGroup( "Application ImGui", true /* omit if the group is empty */ ) );

		Application::RenderImGui();
	}

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox ImGui", true /* omit if the group is empty */ ) );

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	Engine::ImGuiDrawer::Draw( phong_shader );
	Engine::ImGuiDrawer::Draw( light_source_shader );

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		Engine::ImGuiDrawer::Draw( light_source_material_array[ i ] );
	Engine::ImGuiDrawer::Draw( ground_quad_material );
	Engine::ImGuiDrawer::Draw( front_wall_quad_material );
	for( auto& cube_material : cube_material_array )
		Engine::ImGuiDrawer::Draw( cube_material );

	if( ImGui::Begin( "Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			ResetLightingData();
		}

		Engine::ImGuiDrawer::Draw( light_directional.data, "Directional Light Properties" );
		ImGui::Separator();
		ImGui::Checkbox( "Animate (Rotate) Point Light", &light_point_is_animated );
		if( light_point_is_animated )
			ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 15.0f );
		for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		{
			const std::string name( "Point Light # " + std::to_string( i ) + " Properties##");
			if( Engine::ImGuiDrawer::Draw( light_point_array[ i ].data, name.c_str(), light_point_is_animated /* hide position. */ ) )
				light_source_material_array[ i ].Set( "uniform_color", light_point_array[ i ].data.diffuse_and_attenuation_linear.color );
		}
		Engine::ImGuiDrawer::Draw( light_spot.data, "Spot Light Properties" );

		for( auto i = 0; i < CUBE_COUNT; i++ )
		{
			auto& surface_data = cube_surface_data_array[ i ];
			Engine::ImGuiDrawer::Draw( surface_data, ( "Cube #" + std::to_string( i + 1 ) + " Surface Properties" ).c_str() );
		}

		Engine::ImGuiDrawer::Draw( ground_quad_surface_data,	 "Ground Quad Surface Properties" );
		Engine::ImGuiDrawer::Draw( front_wall_quad_surface_data, "Front Wall Surface Properties"  );
	}

	ImGui::End();

	Engine::ImGuiDrawer::Draw( camera_transform, "Main Camera", true /* hide scale. */);

	if( ImGui::Begin( "Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			camera_transform.SetTranslation( 0.0f, 10.0f, -20.0f );
			camera_transform.LookAt( Vector3{ 0.0f, -0.5f, 1.0f }.Normalized() );
			
			camera_is_animated = false;
		}

		ImGui::Checkbox( "Animate (Rotate) Camera", &camera_is_animated );
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
	light_point_is_animated  = true;
	light_point_orbit_radius = 13.25f;

	light_directional = 
	{
		.data =
		{
			.ambient               = Engine::Color3{  0.05f,  0.05f,  0.05f },
			.diffuse               = Engine::Color3{  0.4f,   0.4f,   0.4f  },
			.specular              = Engine::Color3{  0.5f,   0.5f,   0.5f  },
			.direction_view_space  = {  0.2f,  -1.0f,   1.0f  }, // Does not matter, will be updated with the correct view space value every frame.
		/* End of GLSL equivalence. */
			.direction_world_space = {  0.2f,  -1.0f,   1.0f  }
		},
		.transform = &light_directional_transform
	};

	light_point_array = std::vector< Engine::PointLight >( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_point_array[ i ] =
		{
			.data =
			{
				.ambient_and_attenuation_constant = {.color = {  0.05f,  0.05f,  0.05f },							.scalar = 0.075f	},
				.diffuse_and_attenuation_linear   = {.color = Engine::Math::Random::Generate< Engine::Color3 >(),	.scalar = 0.0035f	},
				.specular_attenuation_quadratic   = {.color = {  1.0f,   1.0f,   1.0f  },							.scalar = 0.09f },
				.position_view_space = {  0.2f,  -1.0f,   1.0f  }, // Does not matter, will be updated with the correct view space value every frame.
			/* End of GLSL equivalence. */
				.position_world_space = {  0.2f,  -1.0f,   1.0f  }
			},
			.transform = &light_point_transform_array[ i ]
		};
	}

	light_spot =
	{
		.data =
		{
			.ambient  = Engine::Color3{  0.05f,  0.05f,  0.05f },
			.diffuse  = Engine::Color3{  0.4f,   0.4f,   0.4f  },
			.specular = Engine::Color3{  0.5f,   0.5f,   0.5f  },

			// These two's values do not matter, will be updated with the correct view space value every frame.
			.position_view_space_and_cos_cutoff_angle_inner  = { .vector = {  0.2f,  -1.0f,   1.0f  }, .scalar = Engine::Math::Cos( Radians( 12.5_deg ) ) },
			.direction_view_space_and_cos_cutoff_angle_outer = { .vector = {  0.2f,  -1.0f,   1.0f  }, .scalar = Engine::Math::Cos( Radians( 17.5_deg ) ) },
		/* End of GLSL equivalence. */
			.position_world_space   = camera_transform.GetTranslation(),
			.direction_world_space  = camera_transform.Forward(),
			.cutoff_angle_inner     = 12.5_deg,
			.cutoff_angle_outer     = 17.5_deg
		},
		.transform = &light_spot_transform
	};

	cube_surface_data_array = std::vector< Engine::MaterialData::PhongMaterialData >( CUBE_COUNT, 
	{
		.shininess = 32.0f
	} );

	ground_quad_surface_data = front_wall_quad_surface_data =
	{
		.shininess = 32.0f
	};
}

void SandboxApplication::ResetMaterialData()
{
	light_source_material_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ] = Engine::Material( "Light Source #" + std::to_string( i + 1 ), &light_source_shader );
	
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