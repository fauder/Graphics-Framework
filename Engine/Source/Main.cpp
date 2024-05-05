// Project Includes.
#include "Graphics.h"
#include "ImGuiSetup.h"
#include "Platform.h"

bool ImGui_BeginFullScreenWindow( const char* name, bool* p_open = ( bool* )nullptr, ImGuiWindowFlags flags = 0 )
{
#ifdef IMGUI_HAS_VIEWPORT
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos( viewport->GetWorkPos() );
	ImGui::SetNextWindowSize( viewport->GetWorkSize() );
	ImGui::SetNextWindowViewport( viewport->ID );
#else 
	ImGui::SetNextWindowPos( ImVec2( 0.0f, 0.0f ) );
	ImGui::SetNextWindowSize( ImGui::GetIO().DisplaySize );
#endif
	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
	return ImGui::Begin( name, p_open, flags | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize );
}

void ImGui_EndFullScreenWindow()
{
	ImGui::End();
	ImGui::PopStyleVar();
}

void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
{
	if( ImGui::GetIO().WantCaptureKeyboard )
		return;

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

void DrawImGui()
{
    if( ImGui::Begin( "Test Window" ) )
    {
        ImGui::Text( "Example text." );
    }

	ImGui::End();
}

int main()
{
    Platform::InitializeAndCreateWindow( 800, 600 );
	Platform::ChangeTitle( "TODO: CHANGE PROGRAM TITLE" );
    
    ImGuiSetup::Initialize();

	Platform::SetKeyboardEventCallback(
		[ = ]( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
		{
			OnKeyboardEvent( key_code, key_action, key_mods );
		} );

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

    ImGuiSetup::Shutdown();

    return 0;
}