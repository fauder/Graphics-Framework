#pragma once

// Engine Includes.
#include "Application.h"
#include "Platform.h"

extern Engine::Application* Engine::CreateApplication( Engine::BitFlags< Engine::CreationFlags > );

int main( int argc, char** argv )
{
	Engine::BitFlags< Engine::CreationFlags > flags;

	if( argc > 1 && strcmp( argv[ 1 ], "DISABLE_IMGUI" ) == 0 )
		flags.Set( Engine::CreationFlags::OnStart_DisableImGui );

	auto application = Engine::CreateApplication( flags );
	application->Run();
	delete application;
}
