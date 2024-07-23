#pragma once

// Engine Includes.
#include "ImGuiLog.hpp"
#include "Math/Math.hpp"
#include "Platform.h"
#include "Graphics/GLLogger.h"

namespace Engine
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		virtual void Initialize();
		virtual void Shutdown();

		virtual void Run();

		virtual void Update();

		virtual void Render();

		virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels );
		virtual void DrawImGui();

	protected:
		void FreezeTime()   { time_multiplier = 0.0f; }
		void UnfreezeTime() { time_multiplier = 1.0f; }
		bool TimeIsFrozen() { return Math::IsZero( time_multiplier ); }

	private:
		void OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels );

		void CalculateTimeInformation();

		void RenderImGui_FrameStatistics();

	protected:
		bool display_frame_statistics;

		float time_delta;
		float time_current;
		float time_multiplier;

		float time_sin;
		float time_cos;
		float time_mod_1;
		float time_mod_2_pi;

		bool show_gl_logger;
		GLLogger gl_logger;

	private:
		float time_delta_real;
		float time_previous;
		float time_previous_since_start;
		float time_since_start;
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication();
}
