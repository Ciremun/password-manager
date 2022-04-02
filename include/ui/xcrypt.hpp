#pragma once

#include <string>
#include <stdint.h>
#include "imgui.h"
#include "console/util.h"

void ui_load_passwords(uint8_t *aes_key, ImVector<std::string *> &passwords);
void ui_write_encrypted_passwords(std::string const &str);
