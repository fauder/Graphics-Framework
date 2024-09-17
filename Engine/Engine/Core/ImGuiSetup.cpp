// Engine Includes.
#include "ImGuiSetup.h"
#include "Platform.h"

namespace ImGuiSetup
{
	void Initialize()
	{
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Platform/Renderer backends.
        ImGui_ImplGlfw_InitForOpenGL( reinterpret_cast< GLFWwindow* >( Platform::GetWindowHandle() ), true );
        ImGui_ImplOpenGL3_Init();

        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF( "Asset/Font/JetBrainsMono-Regular.ttf", 18 );
        io.Fonts->Build();
	}

    void Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
    }

    void SetStyle()
    {
        ImGui::StyleColorsDark();

        ImGuiStyle* style = &ImGui::GetStyle();

        style->WindowPadding     = ImVec2( 8, 8 );
        style->WindowRounding    = 5.0f;
        style->FramePadding      = ImVec2( 5, 5 );
        style->FrameRounding     = 4.0f;
        style->ItemSpacing       = ImVec2( 12, 8 );
        style->ItemInnerSpacing  = ImVec2( 8, 6 );
        style->IndentSpacing     = 25.0f;
        style->ScrollbarSize     = 15.0f;
        style->ScrollbarRounding = 9.0f;
        style->GrabMinSize       = 5.0f;
        style->GrabRounding      = 3.0f;
    }
}