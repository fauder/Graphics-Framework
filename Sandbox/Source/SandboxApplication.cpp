// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Core/Assert.h"
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/Platform.h"
#include "Engine/Graphics/Graphics.h"

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

	vertex_array = Engine::VertexArray( vertex_buffer, vertex_buffer_layout, index_buffer );

/* Textures: */
	Engine::Texture::INITIALIZE();

	container_texture.FromFile( R"(Asset/Texture/container.jpg)", GL_RGB );
	awesomeface_texture.FromFile( R"(Asset/Texture/awesomeface.png)", GL_RGBA );

/* Shaders: */
	shader.FromFile( R"(Asset/Shader/Basic.vert)", R"(Asset/Shader/Basic.frag)" );

	shader.Bind();

	shader.SetUniform< int >( "uniform_texture_sampler_container", 0 );
	shader.SetUniform< int >( "uniform_texture_sampler_awesomeface", 1 );

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

	vertex_array.Bind();
	GLCALL( glDrawElements( GL_TRIANGLES, vertex_array.IndexCount(), GL_UNSIGNED_INT, nullptr ) );
}

void SandboxApplication::DrawImGui()
{
	if( ImGui::Begin( "Test Window" ) )
	{
		ImGui::Text( "Running Sandbox Application:\n\nDrawing a rectangle with 2 textures mixed\nwith the new Texture class!" );
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
