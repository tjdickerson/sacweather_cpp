#include <tjd_win32_download.h>
#include "tjd_ui.h"

void InitGui(void* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplOpenGL3_Init();

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF(R"(.\resources\fonts\OpenSans-SemiBold.ttf)", 18.0f);
}


void BeginImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void EndImGui()
{
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void renderSiteCombo(RdaSiteInfo* rdaSites)
{
    s32 selectedItem = 1;
    ImGuiComboFlags flags = 0;

    ImGui::SetNextItemWidth(150.0f);
    if(ImGui::BeginCombo("Sites", "RDA", flags))
    {
        for (int i = 0; i < rdaSites->count; i++)
        {
            ImGui::Selectable(rdaSites->sites[i].name, selectedItem == i);
        }

        ImGui::EndCombo();
    }
}

float menuBarHeight = 0.0f;
float toolBarSize = 40.0f;
void RenderToolbar()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight));
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolBarSize));

	ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoSavedSettings;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::Begin("TOOLBAR", nullptr, window_flags);
	ImGui::PopStyleVar();

	ImGui::PopStyleVar();

	ImGui::SameLine();

	renderSiteCombo(&g_RdaSiteInfo);

	ImGui::End();
}

void InfoPanel()
{
    ImGui::SetNextWindowPos(ImVec2(5, 5));
    ImGui::SetNextWindowSize(ImVec2(175, 15));
    ImGui::SetNextWindowBgAlpha(0.5f);

    ImGuiWindowFlags window_flags = 0
		| ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollbar
		| ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("SiteInfo", nullptr, window_flags);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(ImVec2(5, 5), ImVec2(12, 40), IM_COL32(5, 135, 200, 255));
    ImGui::PopStyleVar(2);

    if (g_CurrentSite != nullptr)
    {
        ImGui::SameLine(15.0f);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s %s", g_CurrentSite->name, g_CurrentSite->displayName);
    }

    ImGui::End();
}

void CheckEvents()
{
    ImGuiIO& io = ImGui::GetIO();
    static RdaSite* closestRda;

    if (io.MouseClicked[1])
    {
        closestRda = FindClosestRdaFromScreen((s32)io.MousePos.x, (s32)io.MousePos.y);
        ImGui::OpenPopup("test");
    }

    if (ImGui::BeginPopupContextWindow("test"))
    {
        if(closestRda != nullptr && ImGui::MenuItem(closestRda->name, "", false))
        {
            StartDownload(closestRda->name, g_CurrentProduct);
        }

        ImGui::Separator();
        if (ImGui::BeginMenu("Products", true))
        {
            for (int i = 0; i < g_NexradProducts->count; i++)
            {
                NexradProduct* product = &g_NexradProducts->nexradProducts[i];
                if(ImGui::MenuItem(product->name, "", false))
                {
                    g_CurrentProduct = GetProductInfo(product->productCode);
                    StartDownload(closestRda->name, g_CurrentProduct);
                    break;
                }
            }

            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
}



void TestIMGUI(bool show_window)
{   
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (show_window)
        ImGui::ShowDemoWindow(&show_window);    

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}