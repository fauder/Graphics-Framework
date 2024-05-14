#pragma once

#include "Application.h"

#include "Platform.h"

extern Engine::Application* Engine::CreateApplication();

int main( int argc, char** argv )
{
	auto application = Engine::CreateApplication();
	application->Run();
	delete application;
}
