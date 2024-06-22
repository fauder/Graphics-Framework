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

// std Includes.
#include <numeric> // std::accumulate().

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
	camera_direction( Engine::Vector3{ 0.0f, -0.5f, 1.0f }.Normalized() ),
	camera_offset( 0.0f, 10.0f, -20.0f ),
	camera_is_animated( false ),
	cube_surface_data( CUBE_COUNT,
	{ 
		.diffuse_map_slot  = 0,
		.specular_map_slot = 1,
		.shininess         = 32.0f 
	} ),
	ground_quad_surface_data
	{
		.diffuse_map_slot  = 0,
		.specular_map_slot = 1,
		.shininess         = 32.0f
	},
	light_directional_data
	{
		.ambient   = {  0.05f,  0.05f,  0.05f },
		.diffuse   = {  0.4f,   0.4f,   0.4f  },
		.specular  = {  0.5f,   0.5f,   0.5f  },
		.direction = {  0.2f,  -1.0f,   1.0f  }
	},
	light_point_data
	{
		.ambient               = {  0.05f,  0.05f,  0.05f },
		.diffuse               = {  0.8f,   0.8f,   0.8f  },
		.specular              = {  1.0f,   1.0f,   1.0f  },
		.position              = {  0.2f,  -1.0f,   1.0f  },
		.attenuation_constant  = 1.0f,
		.attenuation_linear    = 0.09f,
		.attenuation_quadratic = 0.032f,
	},
	light_point_is_animated( true ),
	light_point_orbit_radius( 12.5f ),
	near_plane( 0.1f ), far_plane( 100.0f ),
	aspect_ratio( Platform::GetAspectRatio() ),
	vertical_field_of_view( 90_deg / aspect_ratio ),
	auto_calculate_aspect_ratio( true ),
	auto_calculate_vfov_based_on_90_hfov( true )
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

/* Vertex/Index Data: */
	constexpr auto vertices( Engine::MeshUtility::Interleave( Engine::Primitive::NonIndexed::Cube::Positions,
															  Engine::Primitive::NonIndexed::Cube::Normals,
															  Engine::Primitive::NonIndexed::Cube::UVs ) );

	Engine::VertexBuffer vertex_buffer( vertices.data(), sizeof( vertices ), GL_STATIC_DRAW );
	Engine::VertexBufferLayout vertex_buffer_layout;
	vertex_buffer_layout.Push< float >( 3 ); // Position.
	vertex_buffer_layout.Push< float >( 3 ); // Normal.
	vertex_buffer_layout.Push< float >( 2 ); // UVs.

	vertex_array_crate = Engine::VertexArray( vertex_buffer, vertex_buffer_layout );

/* Textures: */
	Engine::Texture::INITIALIZE();
	
	container_texture_diffuse_map.FromFile( R"(Asset/Texture/container2.png)", GL_RGBA );
	container_texture_specular_map.FromFile( R"(Asset/Texture/container2_specular.png)", GL_RGBA );

/* Shaders: */
	gouraud_shader.FromFile( R"(Asset/Shader/Gouraud.vert)", R"(Asset/Shader/Gouraud.frag)" );
	phong_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	light_source_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/BasicColor.frag)" );

	cube_shader = &phong_shader;

/* View & Projection: */
	aspect_ratio = Platform::GetAspectRatio();

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

void SandboxApplication::Render()
{
	Engine::Application::Render();

	const Engine::Radians current_time_as_angle( Platform::GetCurrentTime() );

	vertex_array_crate.Bind();

	constexpr std::array< Engine::Vector3, CUBE_COUNT > cube_positions =
	{ {
		{  0.0f,  0.0f,  0.0f	},
		{  2.0f,  5.0f, +15.0f	},
		{ -1.5f, -2.2f, +2.5f	},
		{ -3.8f, -2.0f, +12.3f	},
		{  2.4f, -0.4f, +3.5f	},
		{ -1.7f,  3.0f, +7.5f	},
		{  1.3f, -2.0f, +2.5f	},
		{  1.5f,  2.0f, +2.5f	},
		{  1.5f,  0.2f, +1.5f	},
		{ -1.3f,  1.0f, +1.5f	}
	} };

	constexpr Engine::Vector3 cubes_origin = std::accumulate( cube_positions.cbegin(), cube_positions.cend(), Engine::Vector3::Zero() ) / CUBE_COUNT;

	Engine::Matrix4x4 view_transformation;

	/* Camera transform: */
	if( camera_is_animated )
	{
		const auto camera_rotation = Engine::Matrix::RotationAroundY( current_time_as_angle * 0.33f );

		const auto new_camera_offset = ( Engine::Vector4( camera_offset.X(), camera_offset.Y(), camera_offset.Z(), 1.0f ) *
										 ( camera_rotation * Engine::Matrix::Translation( /*camera_offset*/ cubes_origin + Engine::Vector3::Up() * 0.5f ) ) ).XYZ();

		const auto new_camera_direction = camera_direction * camera_rotation.SubMatrix< 3 >();

		view_transformation = Engine::Matrix::LookAt( new_camera_offset, new_camera_direction.Normalized() );
	}
	else
		view_transformation = Engine::Matrix::LookAt( camera_offset, camera_direction.Normalized() );

/* Render the light source: */
	light_source_shader.Bind();

	light_source_shader.SetUniform( "uniform_transform_view", view_transformation );

	/* Light source transform: */
	if( light_point_is_animated )
	{
		const auto point_light_rotation = Engine::Matrix::RotationAroundY( current_time_as_angle * 0.33f );

		light_point_data.position = ( ( Engine::Vector4::Forward() * light_point_orbit_radius ) *
									  ( point_light_rotation * Engine::Matrix::Translation( /*camera_offset*/ cubes_origin ) ) ).XYZ();
		light_point_data.position.SetY( 2.0f );
	}

	/* Light color: */
	light_source_shader.SetUniform( "uniform_transform_world",	Engine::Matrix::Translation( light_point_data.position ) );
	light_source_shader.SetUniform( "uniform_color",			light_point_data.diffuse );

	GLCALL( glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() ) );

/* Render cubes: */
	cube_shader->Bind();

	cube_shader->SetUniform( "uniform_transform_view", view_transformation );

/* Lighting information: */
	{
		// Shaders expect the directional light direction in view space.
		const auto directional_light_original_direction = light_directional_data.direction;
		light_directional_data.direction *= view_transformation.SubMatrix< 3 >();
		cube_shader->SetUniform( "uniform_directional_light_data", light_directional_data );
		light_directional_data.direction = directional_light_original_direction;
	}

	{
		// Shaders expect the point light positions in view space.
		const auto point_light_original_position = light_point_data.position;
		light_point_data.position = ( Engine::Vector4( light_point_data.position.X(), light_point_data.position.Y(), light_point_data.position.Z(), 1.0f ) *
									  view_transformation ).XYZ();
		cube_shader->SetUniform( "uniform_point_light_data", light_point_data );
		light_point_data.position = point_light_original_position;
	}

	container_texture_diffuse_map.ActivateAndUse( 0 );
	container_texture_specular_map.ActivateAndUse( 1 );

	for( auto i = 0; i < CUBE_COUNT; i++ )
	{
		Engine::Degrees angle( 20.0f * i );
		const auto transform( Engine::Matrix::RotationAroundAxis( angle, { 1.0f, 0.3f, 0.5f } ) * Engine::Matrix::Translation( cube_positions[ i ] ) );

		/* Lighting: */
		cube_shader->SetUniform( "uniform_surface_data", cube_surface_data[ i ] );

		/* Transform: */
		cube_shader->SetUniform( "uniform_transform_world", transform );

		glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() );
	}

/* Ground quad: */
	{
		constexpr auto transform( Engine::Matrix::Scaling( 25.0f, 0.01f, 25.0f ) );

		/* Lighting: */
		cube_shader->SetUniform( "uniform_surface_data", ground_quad_surface_data );

		/* Transform: */
		cube_shader->SetUniform( "uniform_transform_world", transform );

		glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() );
	}
}

void SandboxApplication::DrawImGui()
{
	bool view_matrix_is_dirty = false, projection_matrix_is_dirty = false;

	if( ImGui::Begin( "Test Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		ImGui::Text( "Running Sandbox Application:\n\n"
					 
					 "Drawing 2 transforming cubes w& the simplest coloring shader ever,\n"
					 "via using the new Color4 class,\n"
					 "by setting transformation matrices as shader uniforms,\n"
					 "& setting Light & Surface uniform data (structs) via a generic SetUniform< Struct >(),\n"
					 "via using a perspective projection matrix with 4:3 aspect ratio,\n"
					 "90 degrees vertical FoV & near & far plane at 0.1 & 100.0." );
	}

	ImGui::End();

	Engine::ImGuiDrawer::Draw( gouraud_shader );
	Engine::ImGuiDrawer::Draw( phong_shader );
	Engine::ImGuiDrawer::Draw( light_source_shader );

	if( ImGui::Begin( "Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			light_directional_data =
			{
				.ambient   = {  0.05f,  0.05f,  0.05f },
				.diffuse   = {  0.4f,   0.4f,   0.4f  },
				.specular  = {  0.5f,   0.5f,   0.5f  },
				.direction = {  0.2f,  -1.0f,   1.0f  }
			};
			light_point_data =
			{
				.ambient               = {  0.05f,  0.05f,  0.05f },
				.diffuse               = {  0.8f,   0.8f,   0.8f  },
				.specular              = {  1.0f,   1.0f,   1.0f  },
				.position              = {  0.2f,  -1.0f,   1.0f  },
				.attenuation_constant  = 1.0f,
				.attenuation_linear    = 0.09f,
				.attenuation_quadratic = 0.032f,
			};
			light_point_is_animated  = true;
			light_point_orbit_radius = 1.5f;

			for( auto& data : cube_surface_data )
			{
				data = Engine::Lighting::SurfaceData
				{
					.diffuse_map_slot  = 0,
					.specular_map_slot = 1,
					.shininess         = 32.0f
				};
			}

			cube_shader = &phong_shader;
		}

		Engine::ImGuiDrawer::Draw( light_directional_data, "Directional Light Properties" );
		ImGui::Separator();
		ImGui::Checkbox( "Animate (Rotate) Point Light", &light_point_is_animated );
		if( light_point_is_animated )
			ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 15.0f );
		Engine::ImGuiDrawer::Draw( light_point_data, "Point Light Properties", light_point_is_animated /* hide position. */ );

		for( auto i = 0; i < CUBE_COUNT; i++ )
		{
			auto& surface_data = cube_surface_data[ i ];
			Engine::ImGuiDrawer::Draw( surface_data, ( "Cube #" + std::to_string( i + 1 ) + " Surface Properties" ).c_str() );
		}

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

			view_matrix_is_dirty = projection_matrix_is_dirty = true;
		}
	}

	ImGui::End();

	if( ImGui::Begin( "Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			camera_offset        = { 0.0f, 0.0f, -3.0f };
			camera_direction     = Engine::Vector3::Forward();
			
			camera_is_animated   = true;

			view_matrix_is_dirty = true;

		}

		if( Engine::ImGuiDrawer::Draw( camera_offset, "Camera Position" ) )
			view_matrix_is_dirty = true;

		if( Engine::ImGuiDrawer::Draw( camera_direction, "Camera Direction" ) )
		{
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
			near_plane                           = 0.1f;
			far_plane                            = 100.0f;
			aspect_ratio                         = Platform::GetAspectRatio();
			vertical_field_of_view               = 90_deg / aspect_ratio;
			projection_matrix_is_dirty           = true;
		}

		float v_fov_radians = ( float )vertical_field_of_view;

		bool modified = false;

		modified |= ImGui::Checkbox( "Auto-calculate Aspect Ratio", &auto_calculate_aspect_ratio );
		modified |= ImGui::Checkbox( "Auto-calculate Vertical FoV to match 90 degrees Horizontal FoV", &auto_calculate_vfov_based_on_90_hfov );
		/*																Min Value:							Max Value:						Format: */													
		modified |= ImGui::SliderFloat( "Near Plane",	&near_plane,	0.0f,								std::min( 100.0f, far_plane )					); 
		modified |= ImGui::SliderFloat( "Far Plane",	&far_plane,		std::max( near_plane, 10.0f ),		1000.0f											);
		modified |= ImGui::SliderFloat( "Aspect Ratio",	&aspect_ratio,	0.1f,								5.0f											);
		modified |= ImGui::SliderAngle( "Vertical FoV", &v_fov_radians, 1.0f,								180.0f,							"%.2f degrees"	);

		if( modified )
		{
			if( auto_calculate_aspect_ratio )
			{
				aspect_ratio = Platform::GetAspectRatio();
				view_matrix_is_dirty = true;
			}
			
			if( auto_calculate_vfov_based_on_90_hfov )
				vertical_field_of_view = 90_deg / aspect_ratio;
			else
				vertical_field_of_view = Engine::Radians( v_fov_radians );

			projection_matrix_is_dirty = true;
		}
	}

	ImGui::End();

	if( view_matrix_is_dirty )
		UpdateViewMatrix( *cube_shader );

	if( projection_matrix_is_dirty )
		UpdateProjectionMatrix( *cube_shader );
}

void SandboxApplication::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
{
	// Re-calculate the aspect ratio:
	if( auto_calculate_aspect_ratio )
	{
		aspect_ratio = float( width_new_pixels ) / height_new_pixels;
		if( auto_calculate_vfov_based_on_90_hfov )
			vertical_field_of_view = 90_deg / aspect_ratio;

		UpdateProjectionMatrix( light_source_shader );
		UpdateProjectionMatrix( *cube_shader );
	}
}

void SandboxApplication::UpdateViewMatrix( Engine::Shader& shader )
{
	const auto view_transformation = Engine::Matrix::LookAt( camera_offset, camera_direction.Normalized() );
	shader.Bind();
	shader.SetUniform( "uniform_transform_view", view_transformation );
	light_source_shader.Bind();
	light_source_shader.SetUniform( "uniform_transform_view", view_transformation );
}

void SandboxApplication::UpdateProjectionMatrix( Engine::Shader& shader )
{
	//const auto projection_transformation = Engine::Matrix::OrthographicProjection( -16, +16, -9, +9, -10, +10 );
	const auto projection_transformation = Engine::Matrix::PerspectiveProjection( near_plane, far_plane, aspect_ratio, vertical_field_of_view );
	shader.Bind();
	shader.SetUniform( "uniform_transform_projection", projection_transformation );
	light_source_shader.Bind();
	light_source_shader.SetUniform( "uniform_transform_projection", projection_transformation );
}

//void SandboxApplication::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
//{
//	switch( key_code )
//	{
//		case Platform::KeyCode::KEY_ESCAPE:
//			if( key_action == Platform::KeyAction::PRESS )
//				Platform::SetShouldClose( true );
//			break;
//		default:
//			break;
//	}
//}
