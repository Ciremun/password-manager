#include <string>

#include <stdint.h>
#include <limits.h>

#include "stb_sprintf.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "console/util.h"
#include "console/io.h"
#include "console/b64.h"
#include "console/xcrypt.h"
#include "console/rand.h"
#include "ui/update.hpp"
#include "ui/xcrypt.hpp"

#ifdef __ANDROID__
const char* AndroidGetExternalFilesDir();
#endif // __ANDROID__

uint8_t aes_key[32] = {0};
bool password_entered = false;
ImVector<std::string *> passwords;

String sync_remote_url;

extern "C" const char *data_store;

#ifndef __EMSCRIPTEN__
void write_clipboard(const char* c_str)
{
    ImGui::SetClipboardText(c_str);
}
#endif // __EMSCRIPTEN__

void ui_update()
{
    static double last_input_time = 0.0;
    static bool aes_key_changed = false;
    ImGuiIO& io = ImGui::GetIO();
    if (!password_entered)
    {
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
        static bool data_store_initialized = false;
        if (!data_store_initialized)
        {
#ifdef __ANDROID__
            static char android_storage_path[1024] = {0};
            stbsp_snprintf(android_storage_path, 1024, "%s/" DEFAULT_DATA_STORE, AndroidGetExternalFilesDir());
            data_store = android_storage_path;
#else
            data_store = DEFAULT_DATA_STORE;
#endif // __ANDROID__
            passwords.reserve(256);
            data_store_initialized = true;
        }
        else
        {
            for (const auto &password : passwords)
                delete password;
            passwords.clear();
        }
        ui_load_passwords(aes_key, passwords);
    }
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("##io", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (ImGui::BeginTabBar("##tab_bar_first_line", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll))
    {
        // static int last_active_item = -1;
        if (ImGui::BeginTabItem("passwords"))
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            if (ImGui::Button("New Password"))
                passwords.push_back(new std::string());
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
                        delete password;
                        last_input_time = ImGui::GetTime();
                    }
                }
                //
                // Mobile-only copy button
                //
                // if (ImGui::IsItemActive() || last_active_item == i)
                // {
                //     last_active_item = i;
                //     ImGui::SameLine();
                //     if (ImGui::Button("Copy"))
                //         write_clipboard(password->c_str());
                // }
                ImGui::PopID();
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("base64"))
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            static bool use_key_for_b64 = false;
            static std::string base64_encode_bar_str;
            static std::string base64_encode_result;
            static std::string base64_decode_bar_str;
            static std::string base64_decode_result;
            const auto do_base64_encode = [&]() {
                uint8_t *str_to_encode = (uint8_t *)base64_encode_bar_str.c_str();
                char *encoded_str = 0;
                if (use_key_for_b64)
                {
                    uint8_t *str_to_encode_copy = (uint8_t *)malloc(base64_encode_bar_str.length());
                    memcpy(str_to_encode_copy, str_to_encode, base64_encode_bar_str.length());
                    xcrypt_buffer(str_to_encode_copy, aes_key, base64_encode_bar_str.length());
                    encoded_str = b64_encode(str_to_encode_copy, base64_encode_bar_str.length(), 0);
                    free(str_to_encode_copy);
                }
                else
                    encoded_str = b64_encode(str_to_encode, base64_encode_bar_str.length(), 0);
                base64_encode_result = encoded_str;
                free(encoded_str);
            };
            const auto do_base64_decode = [&]() {
                uint8_t *str_to_decode = (uint8_t *)base64_decode_bar_str.c_str();
                size_t decoded_str_length = 0;
                uint8_t *decoded_str = b64_decode(str_to_decode, base64_decode_bar_str.length(), &decoded_str_length);
                if (use_key_for_b64)
                    xcrypt_buffer(decoded_str, aes_key, decoded_str_length);
                base64_decode_result = (char *)decoded_str;
                free(decoded_str);
            };
            if (ImGui::Checkbox("Use Key", &use_key_for_b64))
            {
                if (!base64_encode_bar_str.empty())
                    do_base64_encode();
                if (!base64_decode_bar_str.empty())
                    do_base64_decode();
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::Text("base64 encode");
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            if (ImGui::InputTextWithHint("##base64_encode", "string to encode...", &base64_encode_bar_str) || aes_key_changed)
            {
                do_base64_encode();
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::InputTextWithHint("##base64_encode_result", 0, &base64_encode_result, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::Dummy(ImVec2(0.0f, 12.0f));
            ImGui::Text("base64 decode");
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            if (ImGui::InputTextWithHint("##base64_decode", "string to decode...", &base64_decode_bar_str) || aes_key_changed)
            {
                aes_key_changed = false;
                do_base64_decode();
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::InputTextWithHint("##base64_decode_result", 0, &base64_decode_result, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::EndTabItem();
        }
#ifdef __ANDROID__
        if (ImGui::BeginTabItem("generate"))
#else
        if (ImGui::BeginTabItem("generate password"))
#endif // __ANDROID__
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
#ifdef __ANDROID__
            ImGui::Text("generate password");
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
#endif // __ANDROID__
            static std::string password_name;
#ifdef __ANDROID__
            ImGui::PushItemWidth(io.DisplaySize.x / 1.1f);
#else
            ImGui::PushItemWidth(350.0f);
#endif // __ANDROID__
            ImGui::InputTextWithHint("##password_name", "password name (optional)", &password_name);
            static std::string password_length;
            ImGui::InputTextWithHint("##password_length", "password length (optional)", &password_length, ImGuiInputTextFlags_CharsDecimal);
            ImGui::PopItemWidth();
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            static std::string generated_password;
            if (ImGui::Button("generate"))
            {
                int generated_str_length = 0;
                if (password_length.empty())
                    generated_str_length = random_int();
                else
                {
                    generated_str_length = strtoul(password_length.c_str(), NULL, 10);
                    if (generated_str_length == 0 || !(generated_str_length <= INT_MAX))
                        generated_str_length = random_int();
                }
                uint8_t *generated_str = (uint8_t *)malloc(generated_str_length);
                random_string(generated_str_length, generated_str);
                if (!password_name.empty())
                    generated_password = password_name + " " + (char *)generated_str;
                else
                    generated_password = (char *)generated_str;
                free(generated_str);
            }
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::InputTextWithHint("##generated_password", "result", &generated_password, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("settings"))
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            if (ImGui::Button("Clear Passwords"))
                ImGui::OpenPopup("Clear?");
            ImGui::SameLine();
            if (ImGui::Button("Change Key"))
            {
                memset(aes_key, 0, 32);
                aes_key_changed = true;
                password_entered = false;
            }
            if (ImGui::BeginPopupModal("Clear?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::PushTextWrapPos(io.DisplaySize.x);
                ImGui::TextWrapped("All passwords will be deleted.\nThis operation cannot be undone!\n\n");
                ImGui::PopTextWrapPos();
            #ifdef __ANDROID__
                #define MODAL_BUTTON_SIZE ImVec2(240.0f, 0.0f)
            #else
                #define MODAL_BUTTON_SIZE ImVec2(120.0f, 0.0f)
            #endif // __ANDROID__
                if (ImGui::Button("OK", MODAL_BUTTON_SIZE))
                {
                    for (auto &password : passwords)
                        delete password;
                    passwords.clear();
                    last_input_time = ImGui::GetTime();
                    // last_active_item = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", MODAL_BUTTON_SIZE))
                    ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
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
