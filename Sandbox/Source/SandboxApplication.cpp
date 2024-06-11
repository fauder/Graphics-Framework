// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/Platform.h"
#include "Engine/Graphics/MeshUtility.hpp"
#include "Engine/Graphics/Primitive/Primitive_Cube.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication()
{
    return new SandboxApplication();
}

SandboxApplication::SandboxApplication()
	:
	Engine::Application(),
	cube_1_offset( +1.0f, -1.0f, 0.0f ),
	cube_2_offset( -1.0f, +1.0f, 0.0f ),
	light_source_offset( 0.0f, +1.0f, 0.0f ),
	camera_direction( Engine::Vector3::Forward() ),
	camera_offset( Engine::Vector3::Backward() * 3.0f ),
	cube_1_color( Engine::Color4::Blue() ),
	cube_2_color( Engine::Color4::Yellow() ),
	light_color( Engine::Color4::White() ),
	light_ambient_strength( +0.5f ),
	light_diffuse_strength( +0.5f ),
	near_plane( 0.1f ), far_plane( 100.0f ),
	aspect_ratio( 4.0f / 3.0f ),
	vertical_field_of_view( 90_deg )

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
															  Engine::Primitive::NonIndexed::Cube::Normals ) );

	Engine::VertexBuffer vertex_buffer( vertices.data(), sizeof( vertices ), GL_STATIC_DRAW );
	Engine::VertexBufferLayout vertex_buffer_layout;
	vertex_buffer_layout.Push< float >( 3 ); // Position.
	vertex_buffer_layout.Push< float >( 3 ); // Normal.

	vertex_array_crate = Engine::VertexArray( vertex_buffer, vertex_buffer_layout );

/* Textures: */
	Engine::Texture::INITIALIZE();

/* Shaders: */
	cube_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/Phong.frag)" );
	light_source_shader.FromFile( R"(Asset/Shader/Phong.vert)", R"(Asset/Shader/BasicColor.frag)" );

/* View & Projection: */
	UpdateViewMatrix();
	UpdateProjectionMatrix();

/* Other: */
	//GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) ); // Draw wire-frame.

	GLCALL( glEnable( GL_DEPTH_TEST ) );
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

/* Render cubes: */
	cube_shader.Bind();

	/* Lighting information: */
	cube_shader.SetUniform( "uniform_light_color",		light_color );
	cube_shader.SetUniform( "uniform_ambient_strength", light_ambient_strength );
	cube_shader.SetUniform( "uniform_diffuse_strength", light_diffuse_strength );
	cube_shader.SetUniform( "uniform_light_position",	light_source_offset );

	/* First crate: */
	const auto cube_1_transform( Engine::Matrix::RotationAroundZ( current_time_as_angle ) * Engine::Matrix::Translation( cube_1_offset ) );
	cube_shader.SetUniform( "uniform_transform_world", cube_1_transform );
	cube_shader.SetUniform( "uniform_color", cube_1_color );

	GLCALL( glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() ) );

	/* Second crate: */
	const auto cube_2_transform( /*Engine::Matrix::Scaling( Engine::Math::Abs( Engine::Math::Sin( current_time_as_angle ) ),
													 Engine::Math::Abs( Engine::Math::Cos( current_time_as_angle ) ),
													 1.0f )
							**/ Engine::Matrix::RotationAroundAxis( current_time_as_angle, { 0.707f, 0.707f, 0.0f } )
							* Engine::Matrix::Translation( cube_2_offset ) );
	cube_shader.SetUniform( "uniform_transform_world", cube_2_transform );
	cube_shader.SetUniform( "uniform_color", cube_2_color );

	GLCALL( glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() ) );

/* Render the light source: */
	light_source_shader.Bind();

	/* Light color: */
	light_source_shader.SetUniform( "uniform_color", light_color );

	/* First crate: */
	const auto light_source_transform( Engine::Matrix::Translation( light_source_offset ) );
	light_source_shader.SetUniform( "uniform_transform_world", light_source_transform );

	GLCALL( glDrawArrays( GL_TRIANGLES, 0, vertex_array_crate.VertexCount() ) );
}

void SandboxApplication::DrawImGui()
{
	if( ImGui::Begin( "Test Window", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		ImGui::Text( "Running Sandbox Application:\n\n"
					 
					 "Drawing 2 transforming cubes w& the simplest coloring shader ever,\n"
					 "via using the new Color4 class,\n"
					 "by setting transformation matrices as shader uniforms,\n"
					 "via using a perspective projection matrix with 4:3 aspect ratio,\n"
					 "90 degrees vertical FoV & near & far plane at 0.1 & 100.0." );
	}

	ImGui::End();

	if( ImGui::Begin( "Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			cube_1_color           = Engine::Color4::Blue();
			cube_2_color           = Engine::Color4::Yellow();
			light_color            = Engine::Color4::White();
			light_ambient_strength = +0.5f;
			light_diffuse_strength = +0.5f;
		}

		float cube_1_color_array[ 4 ] = { cube_1_color.R(), cube_1_color.G(), cube_1_color.B(), 1.0f };
		if( ImGui::ColorEdit3( "Cube 1 Color", cube_1_color_array ) )
			cube_1_color.Set( cube_1_color_array );

		float cube_2_color_array[ 4 ] = { cube_2_color.R(), cube_2_color.G(), cube_2_color.B(), 1.0f };
		if( ImGui::ColorEdit3( "Cube 2 Color", cube_2_color_array ) )
			cube_2_color.Set( cube_2_color_array );

		float light_color_array[ 4 ] = { light_color.R(), light_color.G(), light_color.B(), 1.0f };
		if( ImGui::ColorEdit3( "Light Color", light_color_array ) )
			light_color.Set( light_color_array );

		ImGui::SliderFloat( "Ambient Strength", &light_ambient_strength, 0.0f, +1.0f );
		ImGui::SliderFloat( "Diffuse Strength", &light_diffuse_strength, 0.0f, +1.0f );
	}

	ImGui::End();

	if( ImGui::Begin( "Cube & Light Positions", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			cube_1_offset       = { +1.0f, -1.0f, 0.0f };
			cube_2_offset       = { -1.0f, +1.0f, 0.0f };
			light_source_offset = {  0.0f, +1.0f, 0.0f };
		}

		float cube_1_pos_array[ 3 ] = { cube_1_offset.X(), cube_1_offset.Y(), cube_1_offset.Z() };
		if( ImGui::SliderFloat3( "Cube 1 Position", cube_1_pos_array, -5.0f, +5.0f ) )
			cube_1_offset.Set( cube_1_pos_array );

		float cube_2_pos_array[ 3 ] = { cube_2_offset.X(), cube_2_offset.Y(), cube_2_offset.Z() };
		if( ImGui::SliderFloat3( "Cube 2 Position", cube_2_pos_array, -5.0f, +5.0f ) )
			cube_2_offset.Set( cube_2_pos_array );

		float light_source_pos_array[ 3 ] = { light_source_offset.X(), light_source_offset.Y(), light_source_offset.Z() };
		if( ImGui::SliderFloat3( "Light Source Position", light_source_pos_array, -5.0f, +5.0f ) )
			light_source_offset.Set( light_source_pos_array );
	}

	ImGui::End();

	if( ImGui::Begin( "Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			camera_offset = { 0.0f, 0.0f, -3.0f };
			UpdateViewMatrix();
		}

		float camera_pos_array[ 3 ] = { camera_offset.X(), camera_offset.Y(), camera_offset.Z() };
		if( ImGui::SliderFloat3( "Camera Position", camera_pos_array, -10.0f, +10.0f ) )
		{
			camera_offset.Set( camera_pos_array );
			UpdateViewMatrix();
		}

		float camera_rot_array[ 3 ] = { camera_direction.X(), camera_direction.Y(), camera_direction.Z() };
		if( ImGui::SliderFloat3( "Camera Direction", camera_rot_array, -1.0f, +1.0f ) )
		{
			camera_direction.Set( camera_rot_array );
			camera_direction.Normalize();
			UpdateViewMatrix();
		}
	}

	ImGui::End();

	if( ImGui::Begin( "Projection", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Button( "Reset" ) )
		{
			near_plane             = 0.1f;
			far_plane              = 100.0f;
			aspect_ratio           = 4.0f / 3.0f;
			vertical_field_of_view = 90_deg;
			UpdateProjectionMatrix();
		}

		float v_fov_radians = ( float )vertical_field_of_view;

		bool modified = false;

		/*																Min Value:							Max Value:						Format: */													
		modified |= ImGui::SliderFloat( "Near Plane",	&near_plane,	0.0f,								std::min( 100.0f, far_plane )					); 
		modified |= ImGui::SliderFloat( "Far Plane",	&far_plane,		std::max( near_plane, 10.0f ),		1000.0f											);
		modified |= ImGui::SliderFloat( "Aspect Ratio",	&aspect_ratio,	0.1f,								5.0f											);
		modified |= ImGui::SliderAngle( "Vertical FoV", &v_fov_radians, 1.0f,								180.0f,							"%.2f degrees"	);

		if( modified )
		{
			vertical_field_of_view = Engine::Radians( v_fov_radians );
			UpdateProjectionMatrix();
		}
	}

	ImGui::End();
}

void SandboxApplication::UpdateViewMatrix()
{
	const auto view_transformation = Engine::Matrix::LookAt( camera_offset, camera_direction );
	cube_shader.Bind();
	cube_shader.SetUniform( "uniform_transform_view", view_transformation );
	light_source_shader.Bind();
	light_source_shader.SetUniform( "uniform_transform_view", view_transformation );
}

void SandboxApplication::UpdateProjectionMatrix()
{
	const auto projection_transformation = Engine::Matrix::PerspectiveProjection( near_plane, far_plane, aspect_ratio, vertical_field_of_view );
	cube_shader.Bind();
	cube_shader.SetUniform( "uniform_transform_projection", projection_transformation );
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
