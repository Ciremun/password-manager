#include <stdio.h>

#include "console/glob.h"

// NOTE: MSVC C++ compiler does not support compound literals (C99 feature)
// Plain structures in C++ (without constructors) can be initialized with { }
#if defined(__cplusplus)
    #define CLITERAL(type)      type
#else
    #define CLITERAL(type)      (type)
#endif

#define DA_INIT_CAP 256
#define da_append(da, item)                                                          \
    do {                                                                             \
        if ((da)->count >= (da)->capacity) {                                         \
            (da)->capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;   \
            (da)->items = realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy more RAM lol");                       \
        }                                                                            \
                                                                                     \
        (da)->items[(da)->count++] = (item);                                         \
    } while (0)

const char *GLOB_ERROR_UNFINISHED_ESCAPE = "Unfinished escape sequence.";
const char *GLOB_ERROR_UNCLOSED_BLOCK = "Missing closing ] for a character set block.";
const char *GLOB_ERROR_INVALID_ESCAPE = "Invalid escape sequence. Only special characters are allowed to be escaped.";
const char *GLOB_ERROR_UNESCAPED_SPECIAL = "Unescaped special character! To avoid confusion, please, escape it!";

void char_set_add(Char_Set *cs, uint8_t c)
{
    cs->chunks[c/64] |= (1ULL<<(c%64));
}

bool char_set_contains(Char_Set *cs, uint8_t c)
{
    return (cs->chunks[c/64]>>(c%64))&1;
}

static bool is_special(char x)
{
    if (x == '*')  return true;
    if (x == '?')  return true;
    if (x == '\\') return true;
    if (x == '[')  return true;
    if (x == ']')  return true;
    return false;
}

Glob_Result glob_compile(const char *pattern, Glob *glob)
{
    size_t location = 0;
    while (pattern[location] != '\0') {
        switch (pattern[location]) {
            case '*': {
                da_append(glob, CLITERAL(Glob_Cmd) {
                    .kind = CMD_MANY
                });
                location += 1;
            } break;

            case '?': {
                da_append(glob, (CLITERAL(Glob_Cmd) {
                    .kind = CMD_ONE_OF,
                    .cs = {
                        // All of them!
                        .chunks = {
                            0xFFFFFFFFFFFFFFFF,
                            0xFFFFFFFFFFFFFFFF,
                            0xFFFFFFFFFFFFFFFF,
                            0xFFFFFFFFFFFFFFFF,
                        },
                    },
                }));
                location += 1;
            } break;

            case '\\': {
                location += 1;

                if (pattern[location] == '\0') {
                    return CLITERAL(Glob_Result) {
                        .error = GLOB_ERROR_UNFINISHED_ESCAPE,
                        .location = location,
                    };
                }

                if (!is_special(pattern[location])) {
                    return CLITERAL(Glob_Result) {
                        .error = GLOB_ERROR_INVALID_ESCAPE,
                        .location = location,
                    };
                }

                Glob_Cmd cmd = {0};
                cmd.kind = CMD_ONE_OF;
                char_set_add(&cmd.cs, pattern[location]);
                da_append(glob, cmd);
                location += 1;
            } break;

            case '[': {
                location += 1;

                Glob_Cmd cmd = {0};
                cmd.kind = CMD_ONE_OF;

                while (pattern[location] != '\0') {
                    if (pattern[location] == ']') {
                        break;
                    } else if (pattern[location] == '\\') {
                        location += 1;

                        if (pattern[location] == '\0') {
                            return CLITERAL(Glob_Result) {
                                .error = GLOB_ERROR_UNFINISHED_ESCAPE,
                                .location = location,
                            };
                        }

                        if (!is_special(pattern[location])) {
                            return CLITERAL(Glob_Result) {
                                .error = GLOB_ERROR_INVALID_ESCAPE,
                                .location = location,
                            };
                        }

                        char_set_add(&cmd.cs, pattern[location]);
                        location += 1;
                    } else {
                        if (is_special(pattern[location])) {
                            return CLITERAL(Glob_Result) {
                                .error = GLOB_ERROR_UNESCAPED_SPECIAL,
                                .location = location,
                            };
                        } else {
                            char_set_add(&cmd.cs, pattern[location]);
                            location += 1;
                        }
                    }
                }

                if (pattern[location] != ']') {
                    return CLITERAL(Glob_Result) {
                        .error = GLOB_ERROR_UNCLOSED_BLOCK,
                        .location = location,
                    };
                }
                location += 1;

                da_append(glob, cmd);

            } break;

            default: if (is_special(pattern[location])) {
                return CLITERAL(Glob_Result) {
                    .error = GLOB_ERROR_UNESCAPED_SPECIAL,
                    .location = location,
                };
            } else {
                Glob_Cmd cmd = {0};
                cmd.kind = CMD_ONE_OF;
                char_set_add(&cmd.cs, pattern[location]);
                da_append(glob, cmd);

                location += 1;
            }
        }
    }

    return CLITERAL(Glob_Result) {0};
}

bool glob_match(const char *text, size_t text_count, Glob_Cmd *cmds, size_t cmds_count)
{
    while (text_count > 0 && cmds_count > 0) {
        switch (cmds->kind) {
            case CMD_MANY: {
                if (glob_match(text, text_count, cmds + 1, cmds_count - 1)) return true;
                text += 1;
                text_count -= 1;
            } break;

            case CMD_ONE_OF: {
                if (char_set_contains(&cmds->cs, *text)) {
                    text += 1;
                    text_count -= 1;
                    cmds += 1;
                    cmds_count -= 1;
                } else {
                    return false;
                }
            } break;

            default: assert(0 && "unreachable");
        }
    }

    while (cmds_count > 0 && cmds->kind == CMD_MANY) {
        cmds += 1;
        cmds_count -= 1;
    }
    return cmds_count == 0 && text_count == 0;
}

// TODO: implement tests for the glob
