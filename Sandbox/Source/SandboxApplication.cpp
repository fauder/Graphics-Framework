// Sandbox Includes.
#include "SandboxApplication.h"

// Engine Includes.
#include "Engine/Assert.h"
#include "Engine/Graphics.h"
#include "Engine/ImGuiSetup.h"
#include "Engine/Platform.h"

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
	vertices =
	{
	/*	Position:				Color:				UV: */
		 0.5f,  0.5f, 0.0f,		1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // Top Right.
		 0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // Bottom Right.
		-0.5f, -0.5f, 0.0f,		0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // Bottom Left.
		-0.5f,  0.5f, 0.0f,		1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // Top Left.
	};
	indices =
	{
		0, 1, 3, // First triangle.
		1, 2, 3  // Second triangle.
	};

	// Create & bind the vertex array object.
	GLCALL( glGenVertexArrays( 1, &vertex_array_object ) );
	GLCALL( glBindVertexArray( vertex_array_object ) );

	// Create & bind the vertex buffer object.
	unsigned int vertex_buffer_object;
	GLCALL( glGenBuffers( 1, &vertex_buffer_object ) );
	GLCALL( glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer_object ) );
	GLCALL( glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices.data(), GL_STATIC_DRAW ) );

	// Create & bind the element buffer object.
	unsigned int element_buffer_object;
	GLCALL( glGenBuffers( 1, &element_buffer_object ) );
	GLCALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, element_buffer_object ) );
	GLCALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices.data(), GL_STATIC_DRAW ) );

	// Define vertex attribute pointers & enable them.
	constexpr unsigned int vertex_stride = VERTEX_ATTRIBUTE_ELEMENT_COUNT * sizeof( float );
	GLCALL( glVertexAttribPointer( /* vertex attrib. location: */ 0, /* vector3 so -> */ 3, GL_FLOAT, /* Do not normalize */ GL_FALSE, vertex_stride, nullptr ) );
	GLCALL( glEnableVertexAttribArray( 0 ) );
	GLCALL( glVertexAttribPointer( /* vertex attrib. location: */ 1, /* vector3 so -> */ 3, GL_FLOAT, /* Do not normalize */ GL_FALSE, vertex_stride, ( char* )0 + 12 ) );
	GLCALL( glEnableVertexAttribArray( 1 ) );
	GLCALL( glVertexAttribPointer( /* vertex attrib. location: */ 2, /* vector2 so -> */ 2, GL_FLOAT, /* Do not normalize */ GL_FALSE, vertex_stride, ( char* )0 + 24 ) );
	GLCALL( glEnableVertexAttribArray( 2 ) );

	GLCALL( glBindBuffer( GL_ARRAY_BUFFER, 0 ) ); // It is safe to unbind the VBO as the glVertexAttribPointer call above registered the VBO to the VAO.
	
	/* IMPORTANT NOTE REGARDING UNBINDG STUFF BEFORE UNBINDING VAO:
	 * But it is NOT safe to unbind the EBO before unbinding the VAO, as this will cause the VAO to NOT store the EBO anymore.
	 * In the case of VBO, glVertexAttribPointer was the reason we could unbind the VBO before the VAO. */

	GLCALL( glBindVertexArray( 0 ) ); // Unbind to prevent accidentally setting other unwanted staff to VAO.

/* Textures: */
	Engine::Texture::INITIALIZE();

	container_texture.FromFile( R"(Asset/Texture/container.jpg)", GL_RGB );
	awesomeface_texture.FromFile( R"(Asset/Texture/awesomeface.png)", GL_RGBA );

/* Shaders: */
	shader.FromFile( R"(Asset/Shader/Basic.vert)", R"(Asset/Shader/Basic.frag)" );

	shader.Use();

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

	const auto current_time = Platform::GetCurrentTime();

	shader.Use();

	float red_value = ( cos( current_time ) / 2.0f ) + 0.5f;
	float green_value = ( sin( current_time ) / 2.0f ) + 0.5f;

	//shader.SetUniform( "uniform_color", std::array< float, 4 >{ red_value, green_value, 0.0f, 1.0f } );

	container_texture.ActivateAndUse( 0 );
	awesomeface_texture.ActivateAndUse( 1 );

	GLCALL( glBindVertexArray( vertex_array_object ) );
	//GLCALL( glDrawArrays( GL_TRIANGLES, 0, ( int )vertices.size() ) );
	GLCALL( glDrawElements( GL_TRIANGLES, ( int )indices.size(), GL_UNSIGNED_INT, nullptr ) );
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
