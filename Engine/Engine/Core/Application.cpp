// Engine Includes.
#include "Application.h"
#include "Graphics/Graphics.h"
#include "ImGuiSetup.h"

// std Includes.
#include <iostream>

namespace Engine
{
	Application::Application()
	{
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
				this->OnKeyboardEventInternal( key_code, key_action, key_mods );
			} );

		Platform::SetFramebufferResizeCallback(
			[ = ]( const int width_new_pixels, const int height_new_pixels )
			{
				this->OnFramebufferResizeEventInternal( width_new_pixels, height_new_pixels );
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
		GLCALL( glClearColor( 0.1f, 0.1f, 0.1f, 1.0f ) );
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

	void Application::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
	{
	}

	void Application::OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return;

		OnKeyboardEvent( key_code, key_action, key_mods );
	}

	void Application::OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels )
	{
		OnFramebufferResizeEvent( width_new_pixels, height_new_pixels );
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