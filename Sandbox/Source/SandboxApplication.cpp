// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/ImGuiDrawer.hpp"
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/Platform.h"
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
	cube_shader( nullptr ),
	gouraud_shader( "Gouraud" ),
	phong_shader( "Phong" ),
	light_source_shader( "Basic Color" ),
	camera_transform( Vector3::One(), Quaternion::LookRotation( Vector3{ 0.0f, -0.5f, 1.0f }.Normalized() ), Vector3{ 0.0f, 10.0f, -20.0f } ),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
	camera_is_animated( false ),
	auto_calculate_aspect_ratio( true ),
	auto_calculate_vfov_based_on_90_hfov( true ),
	ui_interaction_enabled( false )
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

	//Engine::Math::Random::SeedRandom();

/* Shaders: */
	gouraud_shader.FromFile( R"(Asset/Shader/Gouraud.vert)", R"(Asset/Shader/Gouraud.frag)" );
	phong_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	light_source_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/BasicColor.frag)" );

	cube_shader = &phong_shader;

/* Lighting: */
	ResetLightingData();

/* Materials: */
	ResetMaterialData();
	cube_shader->Bind();
	ground_quad_material.Set( "uniform_surface_data", ground_quad_surface_data );
	front_wall_quad_material.Set( "uniform_surface_data", front_wall_quad_surface_data );
	for( auto i = 0; i < CUBE_COUNT; i++ )
		cube_material_array[ i ].Set( "uniform_surface_data", cube_surface_data_array[ i ] );
	light_source_shader.Bind();
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ].Set( "uniform_color", light_point_data_array[ i ].diffuse );

/* Textures: */
	Engine::Texture::INITIALIZE();

	container_texture_diffuse_map.FromFile( R"(Asset/Texture/container2.png)", GL_RGBA );
	container_texture_specular_map.FromFile( R"(Asset/Texture/container2_specular.png)", GL_RGBA );

/* Vertex/Index Data: */
	constexpr auto vertices( Engine::MeshUtility::Interleave( Engine::Primitive::NonIndexed::Cube::Positions,
															  Engine::Primitive::NonIndexed::Cube::Normals,
															  Engine::Primitive::NonIndexed::Cube::UVs ) );

	vertex_buffer_crate = Engine::VertexBuffer( vertices.data(), sizeof( vertices ), GL_STATIC_DRAW );
	Engine::VertexBufferLayout vertex_buffer_layout;
	vertex_buffer_layout.Push< float >( 3 ); // Position.
	vertex_buffer_layout.Push< float >( 3 ); // Normal.
	vertex_buffer_layout.Push< float >( 2 ); // UVs.

	vertex_array_crate = Engine::VertexArray( vertex_buffer_crate, vertex_buffer_layout );

/* View & Projection: */
	UpdateViewMatrix( gouraud_shader);
	UpdateProjectionMatrix( gouraud_shader);
	UpdateViewMatrix( phong_shader );
	UpdateProjectionMatrix( phong_shader );

/* Other: */
	//GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) ); // Draw wire-frame.

	GLCALL( glEnable( GL_DEPTH_TEST ) );

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
	current_time_as_angle = Radians( time_current );

	/* Camera transform: */
	if( camera_is_animated )
	{
		// Orbit motion:
		camera_transform.SetRotation( Engine::Math::EulerToQuaternion( -current_time_as_angle * 0.33f, 0_rad, 0_rad ) );
		camera_transform.SetTranslation( CUBES_ORIGIN + Vector3::Up() * 0.5f + -camera_transform.Forward() * 30.0f );
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

	UpdateViewMatrix( *cube_shader );
}

void SandboxApplication::Render()
{
	Engine::Application::Render();

	vertex_array_crate.Bind();

/* Render the light source: */
	light_source_shader.Bind();
	light_source_material_array.front().Set( "uniform_transform_view", view_transformation ); // Setting this for the first element is enough as it will stay the same for the rest.

	constexpr Radians angle_increment( Engine::Constants< Radians >::Two_Pi() / LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		auto& light_point_data = light_point_data_array[ i ];

		/* Light source transform: */
		if( light_point_is_animated )
		{
			const auto point_light_rotation = Engine::Matrix::RotationAroundY( current_time_as_angle * 0.33f + angle_increment * ( float )i );

			light_point_data.position_world_space = ( ( Vector4::Forward() * light_point_orbit_radius ) *
													  ( point_light_rotation * Engine::Matrix::Translation( CUBES_ORIGIN ) ) ).XYZ();
			light_point_data.position_world_space.SetY( 2.0f );
		}

		/* Light color: */
		light_source_material_array[ i ].Set( "uniform_transform_world", Engine::Matrix::Translation( light_point_data.position_world_space ) );
		light_source_material_array[ i ].Set( "uniform_color" ); // Use the value stored in the material.

		GLCALL( glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() ) );
	}

/* Render cubes: */
	cube_shader->Bind();

	// TODO: 1) When the Renderer is implemented, maybe make it so that Set() will just update the CPU side buffer & the Renderer will issue SetUniform() calls when it is finally submitting the frame.

	auto& first_cube_material = cube_material_array.front();
	first_cube_material.Set( "uniform_transform_view", view_transformation ); // Setting this for the first element is enough as it will stay the same for the rest.

	/* We can set these for the first cube and do not bother with the rest, as the information is the same for all.*/
	/* Lighting information: */
	{
		/* Shaders expect the lights' position & direction in view space. */

		light_directional_data.direction_view_space = light_directional_data.direction_world_space * view_transformation.SubMatrix< 3 >();
		first_cube_material.Set( "uniform_directional_light_data", light_directional_data );
	
		for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		{
			auto& light_point_data = light_point_data_array[ i ];
			light_point_data.position_view_space = ( Vector4( light_point_data.position_world_space.X(), light_point_data.position_world_space.Y(), light_point_data.position_world_space.Z(), 1.0f ) *
													 view_transformation ).XYZ();
			const std::string uniform_name( "uniform_point_light_data[" + std::to_string( i ) + "]" );
			first_cube_material.Set( uniform_name.c_str(), light_point_data );
		}

		light_spot_data.direction_view_space = light_spot_data.direction_world_space * view_transformation.SubMatrix< 3 >();
		light_spot_data.position_view_space = ( Vector4( light_spot_data.position_world_space.X(), light_spot_data.position_world_space.Y(), light_spot_data.position_world_space.Z(), 1.0f ) *
												 view_transformation ).XYZ();

		// Also need to convert the angles to cosines.
		light_spot_data.cos_cutoff_angle_inner = Engine::Math::Cos( Radians( light_spot_data.cutoff_angle_inner ) );
		light_spot_data.cos_cutoff_angle_outer = Engine::Math::Cos( Radians( light_spot_data.cutoff_angle_outer ) );

		first_cube_material.Set( "uniform_spot_light_data", light_spot_data );
	}

	container_texture_diffuse_map.ActivateAndUse( 0 );
	container_texture_specular_map.ActivateAndUse( 1 );

	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		Degrees angle( 20.0f * i );
		const auto transform( Engine::Matrix::RotationAroundAxis( angle, { 1.0f, 0.3f, 0.5f } ) * Engine::Matrix::Translation( CUBE_POSITIONS[ i ] + Vector3::Up() * 5.0f ) );

		/* Lighting: */
		cube_material_array[ i ].Set( "uniform_surface_data"/*, cube_surface_data_array[ i ]*/ );

		/* Transform: */
		cube_material_array[ i ].Set( "uniform_transform_world", transform );

		glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() );
	}

/* Ground quad: */
	{
		constexpr auto transform( Engine::Matrix::Scaling( 25.0f, 0.01f, 125.0f ) );

		/* Lighting: */
		ground_quad_material.Set( "uniform_directional_light_data" ); // Use the value stored in the material.
		ground_quad_material.Set( "uniform_surface_data" ); // Use the value stored in the material.

		/* Transform: */
		ground_quad_material.Set( "uniform_transform_world", transform );

		glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() );
	}

/* Front wall quad: */
	{
		constexpr auto transform( Engine::Matrix::Scaling( 25.0f, 25.0f, 0.01f ) * Engine::Matrix::TranslationOnZ( 5.0f ) );

		/* Lighting: */
		front_wall_quad_material.Set( "uniform_directional_light_data" ); // Use the value stored in the material.
		front_wall_quad_material.Set( "uniform_surface_data" ); // Use the value stored in the material.

		/* Transform: */
		front_wall_quad_material.Set( "uniform_transform_world", transform );

		glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() );
	}
}

void SandboxApplication::DrawImGui()
{
	Application::DrawImGui();

	bool view_matrix_is_dirty = false, projection_matrix_is_dirty = false;

	Engine::ImGuiDrawer::Draw( gouraud_shader );
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

			cube_shader = &phong_shader;
			ground_quad_material.SetShader( cube_shader );
		}

		Engine::ImGuiDrawer::Draw( light_directional_data, "Directional Light Properties" );
		ImGui::Separator();
		ImGui::Checkbox( "Animate (Rotate) Point Light", &light_point_is_animated );
		if( light_point_is_animated )
			ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 15.0f );
		for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		{
			const std::string name( "Point Light # " + std::to_string( i ) + " Properties##");
			if( Engine::ImGuiDrawer::Draw( light_point_data_array[ i ], name.c_str(), light_point_is_animated /* hide position. */ ) )
				light_source_material_array[ i ].Set( "uniform_color", light_point_data_array[ i ].diffuse );
		}
		Engine::ImGuiDrawer::Draw( light_spot_data, "Spot Light Properties" );

		for( auto i = 0; i < CUBE_COUNT; i++ )
		{
			auto& surface_data = cube_surface_data_array[ i ];
			Engine::ImGuiDrawer::Draw( surface_data, ( "Cube #" + std::to_string( i + 1 ) + " Surface Properties" ).c_str() );
		}

		Engine::ImGuiDrawer::Draw( ground_quad_surface_data,	 "Ground Quad Surface Properties" );
		Engine::ImGuiDrawer::Draw( front_wall_quad_surface_data,  "Front Wall Surface Properties" );

		enum LightingModel
		{
			Gouraud = 0,
			Phong = 1
		};
		ImGui::Text( "Lighting Model" );
		ImGui::SameLine();
		static LightingModel lighting_model = LightingModel::Phong;
		bool lighting_model_changed = false;
		lighting_model_changed |= ImGui::RadioButton( "Gouraud", ( int* )&lighting_model, LightingModel::Gouraud );
		ImGui::SameLine();
		lighting_model_changed |= ImGui::RadioButton( "Phong",	( int* )&lighting_model, LightingModel::Phong );

		if( lighting_model_changed )
		{
			cube_shader = lighting_model == LightingModel::Gouraud ? &gouraud_shader : &phong_shader;
			ground_quad_material.SetShader( cube_shader );

			view_matrix_is_dirty = projection_matrix_is_dirty = true;
		}
	}

	ImGui::End();

	if( Engine::ImGuiDrawer::Draw( camera_transform, "Main Camera" ) )
		view_matrix_is_dirty = true;

	if( ImGui::Begin( "Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			camera_transform.SetTranslation( 0.0f, 10.0f, -20.0f );
			camera_transform.LookAt( Vector3{ 0.0f, -0.5f, 1.0f }.Normalized() );
			
			camera_is_animated = false;

			view_matrix_is_dirty = true;
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
			
			projection_matrix_is_dirty = true;
		}

		if( Engine::ImGuiDrawer::Draw( camera, "Main Camera" ) )
			projection_matrix_is_dirty = true;

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

			projection_matrix_is_dirty = true;
		}
	}

	ImGui::End();

	if( view_matrix_is_dirty )
		UpdateViewMatrix( *cube_shader );

	if( projection_matrix_is_dirty )
		UpdateProjectionMatrix( *cube_shader );
}

void SandboxApplication::UpdateViewMatrix( Engine::Shader& shader )
{
	view_transformation = camera_transform.GetInverseOfFinalMatrix_NoScale();
	shader.Bind();
	shader.SetUniform( "uniform_transform_view", view_transformation );
	light_source_shader.Bind();
	light_source_shader.SetUniform( "uniform_transform_view", view_transformation );
}

void SandboxApplication::UpdateProjectionMatrix( Engine::Shader& shader )
{
	const auto& projection_transformation = camera.GetProjectionMatrix();
	shader.Bind();
	shader.SetUniform( "uniform_transform_projection", projection_transformation );
	light_source_shader.Bind();
	light_source_shader.SetUniform( "uniform_transform_projection", projection_transformation );
}

void SandboxApplication::ResetLightingData()
{
	light_point_is_animated  = true;
	light_point_orbit_radius = 13.25f;

	light_directional_data =
	{
		.ambient               = {  0.05f,  0.05f,  0.05f },
		.diffuse               = {  0.4f,   0.4f,   0.4f  },
		.specular              = {  0.5f,   0.5f,   0.5f  },
		.direction_view_space  = {  0.2f,  -1.0f,   1.0f  }, // Does not matter, will be updated with the correct view space value every frame.
		.direction_world_space = {  0.2f,  -1.0f,   1.0f  }
	};
	light_point_data_array = std::vector< Engine::Lighting::PointLightData >( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_point_data_array[ i ] =
		{
			.ambient               = {  0.05f,  0.05f,  0.05f },
			.diffuse               = Engine::Math::Random::Generate< Engine::Color3 >(),
			.specular              = {  1.0f,   1.0f,   1.0f  },
			.position_view_space   = {  0.2f,  -1.0f,   1.0f  }, // Does not matter, will be updated with the correct view space value every frame.
			.attenuation_constant  = 1.0f,
			.attenuation_linear    = 0.09f,
			.attenuation_quadratic = 0.032f,
			.position_world_space  = {  0.2f,  -1.0f,   1.0f  }
		};
	}
	light_spot_data =
	{
		.ambient                = {  0.05f,  0.05f,  0.05f },
		.diffuse                = {  0.4f,   0.4f,   0.4f  },
		.specular               = {  0.5f,   0.5f,   0.5f  },
		.position_view_space    = {  0.2f,  -1.0f,   1.0f  }, // Does not matter, will be updated with the correct view space value every frame.
		.direction_view_space   = {  0.2f,  -1.0f,   1.0f  }, // Does not matter, will be updated with the correct view space value every frame.
		.cos_cutoff_angle_inner = Engine::Math::Cos( Radians( 12.5_deg ) ),
		.cos_cutoff_angle_outer = Engine::Math::Cos( Radians( 17.5_deg ) ),
		/* End of GLSL equivalence. */
		.position_world_space   = camera_transform.GetTranslation(),
		.direction_world_space  = camera_transform.Forward(),
		.cutoff_angle_inner     = 12.5_deg,
		.cutoff_angle_outer     = 17.5_deg
	};

	cube_surface_data_array = std::vector< Engine::Lighting::SurfaceData >( CUBE_COUNT, 
	{
		.diffuse_map_slot  = 0,
		.specular_map_slot = 1,
		.shininess         = 32.0f
	} );

	ground_quad_surface_data = front_wall_quad_surface_data =
	{
		.diffuse_map_slot  = 0,
		.specular_map_slot = 1,
		.shininess         = 32.0f
	};
}

void SandboxApplication::ResetMaterialData()
{
	light_source_material_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
		light_source_material_array[ i ] = Engine::Material( "Light Source #" + std::to_string( i + 1 ), &light_source_shader );
	
	cube_material_array.resize( CUBE_COUNT );
	for( auto i = 0; i < CUBE_COUNT; i++ )
		cube_material_array[ i ] = Engine::Material( "Cube #" + std::to_string( i + 1 ), cube_shader );
	ground_quad_material     = Engine::Material( "Ground", cube_shader );
	front_wall_quad_material = Engine::Material( "Front Wall", cube_shader );
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
				light_spot_data.cutoff_angle_inner = Engine::Math::Min( light_spot_data.cutoff_angle_inner + 0.33_deg, light_spot_data.cutoff_angle_outer );
			break;
		case Platform::KeyCode::KEY_Y:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot_data.cutoff_angle_inner = Engine::Math::Max( light_spot_data.cutoff_angle_inner - 0.33_deg, 0_deg );
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

		UpdateProjectionMatrix( light_source_shader );
		UpdateProjectionMatrix( *cube_shader );
	}
}