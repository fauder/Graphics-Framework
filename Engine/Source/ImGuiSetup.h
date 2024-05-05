#pragma once

// ImGui includes.
#include "../Vendor/ImGui/imgui.h"
#include "../Vendor/ImGui/imgui_impl_glfw.h"
#include "../Vendor/ImGui/imgui_impl_opengl3.h"

namespace ImGuiSetup
{
    void Initialize();
    void Shutdown();

    void BeginFrame();
    void EndFrame();
}
