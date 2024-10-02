// Engine Includes.
#include "ImGuiSetup.h"
#include "Platform.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace ImGuiSetup
{
	void Initialize()
	{
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Platform/Renderer backends.
        ImGui_ImplGlfw_InitForOpenGL( reinterpret_cast< GLFWwindow* >( Platform::GetWindowHandle() ), true );
        ImGui_ImplOpenGL3_Init();

        AddFonts();

        SetStyle();
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

    void AddFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        float base_font_size = 18.0f;
        float icon_font_size = base_font_size /** 2.0f / 3.0f*/;

        // The ranges array is not copied by the AddFont* functions and is used lazily so it NEEDS TO BE available at the time of building or calling GetTexDataAsRGBA32().

        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 }; // Will not be copied by AddFont* so keep in scope.
        ImFontConfig icons_config;
        icons_config.MergeMode            = true;
        icons_config.PixelSnapH           = true;
        icons_config.GlyphMinAdvanceX     = icon_font_size;
        icons_config.FontDataOwnedByAtlas = false;

        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF( "Asset/Font/JetBrainsMono-Regular.ttf", 18 );
        /* Merge in icons from icon font(s): */
        io.Fonts->AddFontFromFileTTF( "Asset/Font/Font-Awesome/" FONT_ICON_FILE_NAME_FAS, icon_font_size, &icons_config, icons_ranges ); // Merge into JetBrainsMono-Regular (18).

        io.Fonts->Build();
    }

    void SetStyle()
    {
        ImGuiStyle& style = ImGui::GetStyle();

        ImVec4* colors                               = style.Colors;
        colors[ ImGuiCol_Text ]                      = ImVec4( 0.92f, 0.92f, 0.92f, 1.00f );
        colors[ ImGuiCol_TextDisabled ]              = ImVec4( 0.53f, 0.53f, 0.53f, 1.00f );
        colors[ ImGuiCol_WindowBg ]                  = ImVec4( 0.19f, 0.19f, 0.19f, 1.00f );
        colors[ ImGuiCol_ChildBg ]                   = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ ImGuiCol_PopupBg ]                   = ImVec4( 0.08f, 0.08f, 0.08f, 0.94f );
        colors[ ImGuiCol_Border ]                    = ImVec4( 0.37f, 0.37f, 0.41f, 0.50f );
        colors[ ImGuiCol_BorderShadow ]              = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
        colors[ ImGuiCol_FrameBg ]                   = ImVec4( 0.11f, 0.11f, 0.11f, 0.78f );
        colors[ ImGuiCol_FrameBgHovered ]            = ImVec4( 0.32f, 0.24f, 0.00f, 0.75f );
        colors[ ImGuiCol_FrameBgActive ]             = ImVec4( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_TitleBg ]                   = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
        colors[ ImGuiCol_TitleBgActive ]             = ImVec4( 0.31f, 0.31f, 0.31f, 1.00f );
        colors[ ImGuiCol_TitleBgCollapsed ]          = ImVec4( 0.14f, 0.14f, 0.14f, 1.00f );
        colors[ ImGuiCol_MenuBarBg ]                 = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ ImGuiCol_ScrollbarBg ]               = ImVec4( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ ImGuiCol_ScrollbarGrab ]             = ImVec4( 0.25f, 0.25f, 0.25f, 1.00f );
        colors[ ImGuiCol_ScrollbarGrabHovered ]      = ImVec4( 0.41f, 0.41f, 0.41f, 1.00f );
        colors[ ImGuiCol_ScrollbarGrabActive ]       = ImVec4( 0.51f, 0.51f, 0.51f, 1.00f );
        colors[ ImGuiCol_CheckMark ]                 = ImVec4( 0.96f, 0.57f, 0.06f, 0.71f );
        colors[ ImGuiCol_SliderGrab ]                = ImVec4( 0.83f, 0.60f, 0.00f, 0.75f );
        colors[ ImGuiCol_SliderGrabActive ]          = ImVec4( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_Button ]                    = ImVec4( 0.83f, 0.60f, 0.00f, 0.75f );
        colors[ ImGuiCol_ButtonHovered ]             = ImVec4( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_ButtonActive ]              = ImVec4( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_Header ]                    = ImVec4( 0.83f, 0.60f, 0.00f, 0.75f );
        colors[ ImGuiCol_HeaderHovered ]             = ImVec4( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_HeaderActive ]              = ImVec4( 1.00f, 0.78f, 0.10f, 0.75f );
        colors[ ImGuiCol_Separator ]                 = ImVec4( 0.37f, 0.37f, 0.41f, 0.50f );
        colors[ ImGuiCol_SeparatorHovered ]          = ImVec4( 0.10f, 0.40f, 0.75f, 0.78f );
        colors[ ImGuiCol_SeparatorActive ]           = ImVec4( 0.10f, 0.40f, 0.75f, 1.00f );
        colors[ ImGuiCol_ResizeGrip ]                = ImVec4( 0.65f, 0.47f, 0.00f, 0.75f );
        colors[ ImGuiCol_ResizeGripHovered ]         = ImVec4( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_ResizeGripActive ]          = ImVec4( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_TabHovered ]                = ImVec4( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_Tab ]                       = ImVec4( 0.70f, 0.51f, 0.00f, 0.75f );
        colors[ ImGuiCol_TabSelected ]               = ImVec4( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_TabSelectedOverline ]       = ImVec4( 1.00f, 0.73f, 0.00f, 0.00f );
        colors[ ImGuiCol_TabDimmed ]                 = ImVec4( 0.32f, 0.24f, 0.00f, 0.75f );
        colors[ ImGuiCol_TabDimmedSelected ]         = ImVec4( 0.45f, 0.34f, 0.00f, 0.75f );
        colors[ ImGuiCol_TabDimmedSelectedOverline ] = ImVec4( 0.50f, 0.50f, 0.50f, 1.00f );
        colors[ ImGuiCol_PlotLines ]                 = ImVec4( 0.61f, 0.61f, 0.61f, 1.00f );
        colors[ ImGuiCol_PlotLinesHovered ]          = ImVec4( 1.00f, 0.43f, 0.35f, 1.00f );
        colors[ ImGuiCol_PlotHistogram ]             = ImVec4( 0.90f, 0.70f, 0.00f, 1.00f );
        colors[ ImGuiCol_PlotHistogramHovered ]      = ImVec4( 1.00f, 0.60f, 0.00f, 1.00f );
        colors[ ImGuiCol_TableHeaderBg ]             = ImVec4( 0.35f, 0.35f, 0.35f, 1.00f );
        colors[ ImGuiCol_TableBorderStrong ]         = ImVec4( 0.27f, 0.27f, 0.27f, 1.00f );
        colors[ ImGuiCol_TableBorderLight ]          = ImVec4( 0.27f, 0.27f, 0.27f, 1.00f );
        colors[ ImGuiCol_TableRowBg ]                = ImVec4( 0.00f, 0.00f, 0.00f, 0.00f );
        colors[ ImGuiCol_TableRowBgAlt ]             = ImVec4( 1.00f, 1.00f, 1.00f, 0.06f );
        colors[ ImGuiCol_TextLink ]                  = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
        colors[ ImGuiCol_TextSelectedBg ]            = ImVec4( 0.26f, 0.59f, 0.98f, 0.35f );
        colors[ ImGuiCol_DragDropTarget ]            = ImVec4( 1.00f, 1.00f, 0.00f, 0.90f );
        colors[ ImGuiCol_NavHighlight ]              = ImVec4( 0.26f, 0.59f, 0.98f, 1.00f );
        colors[ ImGuiCol_NavWindowingHighlight ]     = ImVec4( 1.00f, 1.00f, 1.00f, 0.70f );
        colors[ ImGuiCol_NavWindowingDimBg ]         = ImVec4( 0.80f, 0.80f, 0.80f, 0.20f );
        colors[ ImGuiCol_ModalWindowDimBg ]          = ImVec4( 0.80f, 0.80f, 0.80f, 0.35f );

        /* Main: */
        style.WindowPadding     = ImVec2( 8, 8 );
        style.FramePadding      = ImVec2( 4, 3 );
        style.ItemSpacing       = ImVec2( 8, 4 );
        style.ItemInnerSpacing  = ImVec2( 4, 4 );
        style.IndentSpacing     = 20.0f;
        style.ScrollbarSize     = 16.0f;
        style.GrabMinSize       = 12.0f;

        /* Borders: */
        style.WindowBorderSize   = 0.0f;
        style.ChildBorderSize    = 0.0f;
        style.PopupBorderSize    = 0.0f;
        style.FrameBorderSize    = 0.0f;
        style.TabBorderSize      = 0.0f;
        style.TabBarBorderSize   = 2.0f;
        style.TabBarOverlineSize = 0.0f;

        /* Rounding: */
        style.WindowRounding    = 4.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;

        /* Tables: */
        style.CellPadding                 = ImVec2( 4, 2 );
        style.TableAngledHeadersAngle     = 35.0f;
        style.TableAngledHeadersTextAlign = ImVec2( 0.5f, 0.0f );

        /* Widgets: */
        style.WindowTitleAlign        = ImVec2( 0.0f, 0.5f );
        style.ButtonTextAlign         = ImVec2( 0.5f, 0.5f );
        style.SeparatorTextBorderSize = 2.0f;
        style.SeparatorTextAlign      = ImVec2( 0.0f, 0.5f );
        style.SeparatorTextPadding    = ImVec2( 4, 2 );

        /* Disable collapse button: */
        style.WindowMenuButtonPosition = ImGuiDir_None;
    }
}