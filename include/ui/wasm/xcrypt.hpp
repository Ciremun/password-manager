#pragma once

#include <stdint.h>

#include "imgui.h"

#include "console/util.h"

void ui_encrypt_and_append(String s, uint8_t *aes_key);
void ui_load_passwords(uint8_t *aes_key, ImVector<String> &passwords);
