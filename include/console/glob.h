#ifndef PM_GLOB_H_
#define PM_GLOB_H_

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint64_t chunks[4];
} Char_Set;

void char_set_add(Char_Set *cs, uint8_t c);
bool char_set_contains(Char_Set *cs, uint8_t c);

typedef enum {
    CMD_MANY,
    CMD_ONE_OF,
} Glob_Cmd_Kind;

typedef struct {
    Glob_Cmd_Kind kind;
    Char_Set cs;
} Glob_Cmd;

typedef struct {
    Glob_Cmd *items;
    size_t count;
    size_t capacity;
} Glob;

// You can compare Glob_Result.error pointer to any of these to check
// for a specific error. Usually you just want to print Glob_Result.error
// to the user.
extern const char *GLOB_ERROR_UNFINISHED_ESCAPE;
extern const char *GLOB_ERROR_UNCLOSED_BLOCK;
extern const char *GLOB_ERROR_INVALID_ESCAPE;
extern const char *GLOB_ERROR_UNESCAPED_SPECIAL;

typedef struct {
    const char *error;
    size_t location;
} Glob_Result;

Glob_Result glob_compile(const char *pattern, Glob *glob);
bool glob_match(const char *text, size_t text_count, Glob_Cmd *cmds, size_t cmds_count);

#endif // PM_GLOB_H_
