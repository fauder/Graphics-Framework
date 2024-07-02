#pragma once

// Engine Includes.
#include "Platform.h"

namespace Engine
{
	class _declspec( dllexport ) Application
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

	private:
		void OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels );
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication();
}
