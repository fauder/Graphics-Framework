// Engine Includes.
#include "Application.h"
#include "Graphics/Graphics.h"
#include "ImGuiSetup.h"

#include "Math/Vector.hpp"

// std Includes.
#include <iostream>

namespace Engine
{
	Application::Application()
	{
		constexpr Math::Vector< float, 4 > vector4( 1.0f, 2.0f, 3.0f, 4.0f );
		constexpr Math::Vector< float, 4 > vector4_2( 11.0f, 2.0f, 3.0f, 4.0f );
		constexpr Math::Vector< float, 2 > vector2( 1.0f );
		constexpr Math::Vector< float, 3 > vector3( 1.0f, 0.0f, 0.0f );
		constexpr Math::Vector< float, 3 > vector3_2( 0.0f, 1.0f, 0.0f );

		constexpr auto crossed = Math::Cross( vector3, vector3_2 );

		Initialize();
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		Platform::InitializeAndCreateWindow( 800, 600 );
		Platform::ChangeTitle( "Graphics Framework" );

		GLCALL( const auto version = glGetString( GL_VERSION ) );
		std::cout << version << "\n\n";

		ImGuiSetup::Initialize();

		Platform::SetKeyboardEventCallback(
			[ = ]( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
		{
			std::bind( &Application::OnKeyboardEventInternal, this, key_code, key_action, key_mods );
		} );
	}

	void Application::Shutdown()
	{
		ImGuiSetup::Shutdown();
	}

	void Application::Run()
	{
		/* The render loop. */
		while( !Platform::ShouldClose() )
		{
			Platform::PollEvents();

			Render();

			ImGuiSetup::BeginFrame();
			DrawImGui();
			ImGuiSetup::EndFrame();

			Platform::SwapBuffers();
		}
	}

	void Application::Render()
	{
		GLCALL( glClearColor( 0.55f, 0.55f, 0.55f, 1.0f ) );
		GLCALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );
	}

	void Application::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		switch( key_code )
		{
			case Platform::KeyCode::KEY_ESCAPE:
				/*if( key_action == Platform::KeyAction::PRESS )
					Platform::SetShouldClose( true );*/
				break;
			default:
				break;
		}
	}

	void Application::OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return;

		OnKeyboardEvent( key_code, key_action, key_mods );
	}

	void Application::DrawImGui()
	{
		if( ImGui::Begin( "Test Window" ) )
		{
			ImGui::Text( "Running graphics framework base code." );
		}

		ImGui::End();
	}
}