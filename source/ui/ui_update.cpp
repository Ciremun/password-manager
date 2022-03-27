#include <string>

#include <stdint.h>

#include "stb_sprintf.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "console/util.h"
#include "console/io.h"
#include "console/b64.h"
#include "console/xcrypt.h"
#include "ui/update.hpp"
#include "ui/xcrypt.hpp"

#ifdef __ANDROID__
const char* AndroidGetExternalFilesDir();
#endif // __ANDROID__

uint8_t aes_key[32] = {0};
bool password_entered = false;
ImVector<std::string *> passwords;

String sync_remote_url;

extern const char* data_store;

#ifndef __EMSCRIPTEN__
void write_clipboard(const char* c_str)
{
    printf("set clipboard text!\n");
    ImGui::SetClipboardText(c_str);
}
#endif // __EMSCRIPTEN__

void ui_update()
{
    static double last_input_time = 0.0;

    if (!password_entered)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
#ifdef __ANDROID__
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 240.0f), ImGuiCond_Always);
#else
        ImGui::SetNextWindowSize(ImVec2(400.0f, 70.0f), ImGuiCond_Always);
#endif // __ANDROID__
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
        char test_str[] = "test";
        ui_encrypt_and_append((String) { .data = (uint8_t *)test_str, .length = sizeof(test_str) - 1 }, aes_key);
        passwords.reserve(256);
        ui_load_passwords(aes_key, passwords);
    }
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##io", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTabBar("##tab_bar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
    {
        static int last_active_item = -1;
        if (ImGui::BeginTabItem("passwords"))
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            if (ImGui::Button("New Password"))
                passwords.push_back(new std::string());
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                for (auto &pw : passwords)
                    free(pw);
                passwords.clear();
                last_input_time = ImGui::GetTime();
                last_active_item = -1;
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            static std::string search_bar_str;
            ImGui::InputTextWithHint("##search", "Search", &search_bar_str);
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            for (int i = 0; i < passwords.size(); ++i)
            {
                std::string *password = passwords[i];
                if (!search_bar_str.empty() && !(password->rfind(search_bar_str, 0) == 0))
                    continue;
                ImGui::PushID(i);
                if (ImGui::InputText("", password))
                    last_input_time = ImGui::GetTime();
                if (ImGui::IsItemFocused() && (ImGui::IsKeyPressedMap(ImGuiKey_Enter) || ImGui::IsKeyPressedMap(ImGuiKey_KeyPadEnter)))
                {
                    if (password->empty())
                    {
                        passwords.find_erase(password);
                        free(password);
                        last_input_time = ImGui::GetTime();
                    }
                }
                if (ImGui::IsItemActive() || last_active_item == i)
                {
                    last_active_item = i;
                    ImGui::SameLine();
                    if (ImGui::Button("Copy"))
                        write_clipboard(password->c_str());
                }
                ImGui::PopID();
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
    ImGui::PopStyleVar();

    if (last_input_time)
    {
        double sec_since_input_end = ImGui::GetTime() - last_input_time;
        if (sec_since_input_end > 0.5)
        {
            std::string encrypted_passwords;
            for (auto &pw : passwords)
            {
                uint8_t *pw_copy = (uint8_t *)malloc(pw->length());
                memcpy(pw_copy, pw->c_str(), pw->length());
                xcrypt_buffer(pw_copy, aes_key, pw->length());
                char *enc_pw = b64_encode(pw_copy, pw->length(), 0);
                encrypted_passwords += enc_pw;
                encrypted_passwords += '\n';
                free(enc_pw);
                free(pw_copy);
            }
            ui_write_encrypted_passwords(encrypted_passwords);
            last_input_time = 0.0;
        }
    }
}
