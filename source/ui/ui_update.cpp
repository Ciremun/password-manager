#include "ui/update.hpp"
#include "stb_sprintf.h"

void ui_update()
{
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("pm", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        static char buf[256];
        static char iname[16];
        for (int i = 0; i < 256; ++i)
        {
            stbsp_snprintf(iname, 16, "##%d", i);
            ImGui::InputText(iname, buf, 256);
        }
    ImGui::End();
    ImGui::PopStyleVar();
}
