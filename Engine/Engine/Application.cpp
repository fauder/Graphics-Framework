// Engine Includes.
#include "Application.h"
#include "Graphics.h"
#include "ImGuiSetup.h"

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

			GLCALL( glClearColor( 0.55f, 0.55f, 0.55f, 1.0f ) );
			GLCALL( glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) );

			ImGuiSetup::BeginFrame();

			DrawImGui();

			ImGuiSetup::EndFrame();

			Platform::SwapBuffers();
		}
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