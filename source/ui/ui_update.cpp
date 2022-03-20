#include <stdint.h>

#include "stb_sprintf.h"
#include "imgui_internal.h"

#include "console/util.h"
#include "console/io.h"
#include "ui/update.hpp"
#include "ui/xcrypt.hpp"

static uint8_t aes_key[32] = {0};
static bool password_entered = false;
static ImVector<String> passwords;

String sync_remote_url;

extern const char* data_store;

const char* AndroidGetExternalFilesDir();

void ui_update()
{
    if (!password_entered)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(400.0f, 70.0f), ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Enter password", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
            ImGui::PushItemWidth(ImGui::GetWindowWidth());
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0))
                ImGui::SetKeyboardFocusHere(0);
            ImGui::SetCursorPos(ImVec2(0.0f, ImGui::GetWindowHeight() * 0.5f));
            if (ImGui::InputText("##aes_key", (char *)aes_key, 32, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_Password) && aes_key[0])
                password_entered = true;
            ImGui::PopItemWidth();
        ImGui::End();
        ImGui::PopStyleVar();
        if (!password_entered)
            return;
#ifdef __ANDROID__
        static char android_storage_path[1024] = {0};
        stbsp_snprintf(android_storage_path, 1024, "%s/" DEFAULT_DATA_STORE, AndroidGetExternalFilesDir());
        data_store = android_storage_path;
#else
        data_store = DEFAULT_DATA_STORE;
#endif // __ANDROID__
        char test_str[14] = "test password";
        ui_encrypt_and_append((String) { .data = (uint8_t *)test_str, .length = 13 }, aes_key);
        passwords.reserve(256);
        ui_load_passwords(aes_key, passwords);
    }
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##io", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        for (int i = 0; i < passwords.size(); ++i)
        {
            const auto &password = passwords[i];
            ImGui::PushID(i);
            ImGui::InputText("", (char *)password.data, password.length + 1);
            ImGui::PopID();
        }
    ImGui::End();
    ImGui::PopStyleVar();
}
