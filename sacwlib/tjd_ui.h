//

#ifndef __tjd_ui_h__

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl3.h"
#include "sacw_api.h"

void InitGui(void* window);
void BeginImGui();
void EndImGui();
void RenderToolbar();
void CheckEvents();
void InfoPanel();

#define __tjd_ui_h__
#endif