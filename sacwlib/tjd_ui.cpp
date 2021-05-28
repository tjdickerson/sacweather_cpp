#include "tjd_ui.h"


void InitGui(void* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplOpenGL3_Init();
}

void TestIMGUI(bool show_window)
{   
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (show_window)
        ImGui::ShowDemoWindow(&show_window);    

    // ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}