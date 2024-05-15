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
}

void SandboxApplication::DrawImGui()
{
	if( ImGui::Begin( "Test Window" ) )
	{
		ImGui::Text( "Running Sandbox Application." );
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
