// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/Platform.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"

Engine::Application* Engine::CreateApplication()
{
    return new SandboxApplication();
}

SandboxApplication::SandboxApplication()
	:
	Engine::Application()
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
	constexpr const int vertex_attribute_element_count = 3 + 3 + 2;

	std::array< float, 4 * vertex_attribute_element_count > vertices
	{
	/*	Position:				Color:				UV: */
		 0.5f,  0.5f, 0.0f,		1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // Top Right.
		 0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // Bottom Right.
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // Bottom Left.
		-0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // Top Left.
	};
	std::array< unsigned int, 6 > indices
	{
		0, 1, 3, // First triangle.
		1, 2, 3  // Second triangle.
	};

	Engine::VertexBuffer vertex_buffer( vertices.data(), sizeof( vertices ), GL_STATIC_DRAW );
	Engine::VertexBufferLayout vertex_buffer_layout;
	vertex_buffer_layout.Push< float >( 3 ); // Position.
	vertex_buffer_layout.Push< float >( 3 ); // Color.
	vertex_buffer_layout.Push< float >( 2 ); // UV.
	Engine::IndexBuffer index_buffer( indices.data(), sizeof( indices ), GL_STATIC_DRAW );

	vertex_array_crate = Engine::VertexArray( vertex_buffer, vertex_buffer_layout, index_buffer );

/* Textures: */
	Engine::Texture::INITIALIZE();

	container_texture.FromFile( R"(Asset/Texture/container.jpg)", GL_RGB );
	awesomeface_texture.FromFile( R"(Asset/Texture/awesomeface.png)", GL_RGBA );

/* Shaders: */
	shader.FromFile( R"(Asset/Shader/Basic.vert)", R"(Asset/Shader/Basic.frag)" );

	shader.Bind();

	shader.SetUniform< int >( "uniform_texture_sampler_container",   0 );
	shader.SetUniform< int >( "uniform_texture_sampler_awesomeface", 1 );

/* View & Projection: */
	/* To simulate the Camera going backward, the world should move forward instead. */
	constexpr auto view_transformation = Engine::Matrix::TranslationOnZ( +1.0f ); // Move the camera back on Z axis by 1 unit.
	shader.SetUniform( "uniform_transform_view", view_transformation );

	const auto projection_transformation = Engine::Matrix::PerspectiveProjection( 0.1f, 100.0f, 800.0f / 600.0f, Engine::Radians( Engine::Constants< float >::Pi_Over_Two() ) );
	shader.SetUniform( "uniform_transform_projection", projection_transformation );

/* Other: */
	//GLCALL( glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) ); // Draw wire-frame.
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

	//const auto current_time = Platform::GetCurrentTime();

	shader.Bind();

	container_texture.ActivateAndUse( 0 );
	awesomeface_texture.ActivateAndUse( 1 );

	const Engine::Radians current_time_as_angle( Platform::GetCurrentTime() );

	vertex_array_crate.Bind();

	/* First crate: */

	const auto transform( Engine::Matrix::RotationAroundZ( current_time_as_angle ) * Engine::Matrix::Translation( 0.5f, -0.5f, 0.0f ) );
	shader.SetUniform( "uniform_transform_world", transform );

	GLCALL( glDrawElements( GL_TRIANGLES, vertex_array_crate.IndexCount(), GL_UNSIGNED_INT, nullptr ) );

	/* Second crate: */
	const auto transform_2( Engine::Matrix::Scaling( Engine::Math::Sin( current_time_as_angle ), Engine::Math::Cos( current_time_as_angle ), 1.0f )
						  * Engine::Matrix::Translation( -0.5f, 0.5f, 0.0f ) );
	shader.SetUniform( "uniform_transform_world", transform_2 );

	GLCALL( glDrawElements( GL_TRIANGLES, vertex_array_crate.IndexCount(), GL_UNSIGNED_INT, nullptr ) );
}

void SandboxApplication::DrawImGui()
{
	if( ImGui::Begin( "Test Window" ) )
	{
		ImGui::Text( "Running Sandbox Application:\n\n"
					 
					 "Drawing 2 transforming rectangles each with 2 textures mixed,\n"
					 "by setting transformation matrices as shader uniforms,\n"
					 "via using a perspective projection matris with 4:3 aspect ratio,\n"
					 "90 degrees vertical FoV & near & far plane at 0.1 & 100.0." );
	}

	ImGui::End();
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
