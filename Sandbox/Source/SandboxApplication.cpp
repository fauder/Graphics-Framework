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

	// Set-up triangle data.
	//float rectangle_vertices[] = {
	//	 0.5f,  0.5f, 0.0f, // Top right.
	//	 0.5f, -0.5f, 0.0f, // Bottom right.
	//	-0.5f, -0.5f, 0.0f, // Bottom left.
	//	-0.5f,  0.5f, 0.0f  // Top left.
	//};
	vertices = {
		/* Position:			Color: */
		 0.5f, -0.5f, 0.0f,		1.0f, 0.0f, 0.0f,	// Bottom right.
		-0.5f, -0.5f, 0.0f,		0.0f, 1.0f, 0.0f,	// Bottom left.
		 0.0f,  0.5f, 0.0f,		0.0f, 0.0f, 1.0f	// Top right.
	};
	indices =
	{
		0, 1, 3, // First triangle.
		//1, 2, 3  // Second triangle.
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
	constexpr unsigned int vertex_stride = ( 3 + 3 ) * sizeof( float );
	GLCALL( glVertexAttribPointer( /* vertex attrib. location: */ 0, /* vector3 so -> */ 3, GL_FLOAT, /* Do not normalize */ GL_FALSE, vertex_stride, nullptr ) );
	GLCALL( glEnableVertexAttribArray( 0 ) );
	GLCALL( glVertexAttribPointer( /* vertex attrib. location: */ 1, /* vector3 so -> */ 3, GL_FLOAT, /* Do not normalize */ GL_FALSE, vertex_stride, ( char* )0 + 12 ) );
	GLCALL( glEnableVertexAttribArray( 1 ) );

	GLCALL( glBindBuffer( GL_ARRAY_BUFFER, 0 ) ); // It is safe to unbind the VBO as the glVertexAttribPointer call above registered the VBO to the VAO.
	
	/* IMPORTANT NOTE REGARDING UNBINDG STUFF BEFORE UNBINDING VAO:
	 * But it is NOT safe to unbind the EBO before unbinding the VAO, as this will cause the VAO to NOT store the EBO anymore.
	 * In the case of VBO, glVertexAttribPointer was the reason we could unbind the VBO before the VAO. */

	GLCALL( glBindVertexArray( 0 ) ); // Unbind to prevent accidentally setting other unwanted staff to VAO.

	shader.FromFile( R"(Asset/Shader/Basic.vert)", R"(Asset/Shader/Basic.frag)" );

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

	shader.SetUniform( "final_color", std::array< float, 4 >{ red_value, green_value, 0.0f, 1.0f } );

	GLCALL( glBindVertexArray( vertex_array_object ) );
	//GLCALL( glDrawArrays( GL_TRIANGLES, 0, ( int )vertices.size() ) );
	GLCALL( glDrawElements( GL_TRIANGLES, ( int )indices.size(), GL_UNSIGNED_INT, nullptr ) );
}

void SandboxApplication::DrawImGui()
{
	if( ImGui::Begin( "Test Window" ) )
	{
		ImGui::Text( "Running Sandbox Application:\n\nDrawing a colorful triangle\nwith the new Shader class!" );
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
