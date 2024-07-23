// Engine Includes.
#include "Application.h"
#include "Graphics/Graphics.h"
#include "ImGuiSetup.h"
#include "ImGuiUtility.h"
#include "Math/Math.hpp"

// std Includes.
#include <iostream>

namespace Engine
{
	Application::Application()
		:
		display_frame_statistics( true ),
		time_current( 0.0f ),
		time_multiplier( 1.0f ),
		show_gl_logger( true ),
		time_previous( 0.0f ),
		time_previous_since_start( 0.0f ),
		time_since_start( 0.0f )
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

		const auto version = glGetString( GL_VERSION );
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

		Platform::SetGLDebugOutputCallback( gl_logger.GetCallback() );
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
			CalculateTimeInformation();

			Platform::PollEvents();

			Update();

			Render();

			ImGuiSetup::BeginFrame();
			DrawImGui();
			ImGuiSetup::EndFrame();

			Platform::SwapBuffers();
		}
	}

	void Application::Update()
	{
	}

	void Application::Render()
	{
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

	void Application::CalculateTimeInformation()
	{
		time_since_start = Platform::GetCurrentTime();

		time_delta_real = time_since_start - time_previous_since_start;

		time_current += time_delta_real * time_multiplier;

		time_delta = time_current - time_previous;

		time_previous = time_current;
		time_previous_since_start = time_since_start;

		time_sin = Math::Sin( Radians( time_current ) );
		time_cos = Math::Cos( Radians( time_current ) );
		time_mod_1 = std::fmod( time_current, 1.0f );
		time_mod_2_pi = std::fmod( time_current, Constants< float >::Two_Pi() );
	}

	void Application::DrawImGui()
	{
		RenderImGui_FrameStatistics();

		if( show_gl_logger )
			gl_logger.Draw( &show_gl_logger );
	}

	void Application::RenderImGui_FrameStatistics()
	{
		ImGuiUtility::SetNextWindowPos( ImGuiUtility::HorizontalWindowPositioning::RIGHT, ImGuiUtility::VerticalWindowPositioning::TOP );
		if( ImGui::Begin( "Frame Statistics", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::Text( "FPS: %.1f fps", 1.0f / time_delta_real );
			ImGui::Text( "Delta time (multiplied): %.3f ms | Delta time (real): %.3f", time_delta * 1000.0f, time_delta_real * 1000.0f );
			ImGui::Text( "Time since start: %.3f.", time_since_start );
			ImGui::SliderFloat( "Time Multiplier", &time_multiplier, 0.01f, 5.0f, "x %.2f", ImGuiSliderFlags_Logarithmic ); ImGui::SameLine(); if( ImGui::Button( "Reset##time_multiplier" ) ) time_multiplier = 1.0f;
			if( !TimeIsFrozen() && ImGui::Button( "Pause" ) )
				FreezeTime();
			else if( TimeIsFrozen() && ImGui::Button( "Resume" ) )
				UnfreezeTime();

			auto sin_time = time_sin;
			auto cos_time = time_cos;
			auto time_mod_1 = std::fmod( time_current, 1.0f );

			ImGui::ProgressBar( time_mod_1, ImVec2( 0.0f, 0.0f ) ); ImGui::SameLine(); ImGui::TextUnformatted( "Time % 1" );
			ImGui::ProgressBar( time_mod_2_pi / Constants< float >::Two_Pi(), ImVec2( 0.0f, 0.0f ) ); ImGui::SameLine(); ImGui::TextUnformatted( "Time % (2 * Pi)" );
			ImGui::SliderFloat( "Sin(Time) ", &sin_time, -1.0f, 1.0f, "%.1f", ImGuiSliderFlags_NoInput );
			ImGui::SliderFloat( "Cos(Time) ", &cos_time, -1.0f, 1.0f, "%.1f", ImGuiSliderFlags_NoInput );
		}

		ImGui::End();
	}
}