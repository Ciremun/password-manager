#ifndef PM_UI_H_
#define PM_UI_H_

#include <stddef.h>
#include <stdint.h>

#include "core/pm_util.h"
#include "rawdraw/rd_util.h"

#define RD_INPUT_FIELD_HEIGHT 40
#define RD_INPUT_FIELD_MARGIN 5
#define RD_INPUT_FIELD_FONT_SIZE 4

typedef struct InputField InputField;

typedef struct
{
    String string;
    uint32_t color;
    int font_size;
} Text;

typedef struct
{
    Point p1;
    Point p2;
    uint32_t color;
} Rect;

struct InputField
{
    Rect rect;
    Text text;
    int focused;
    void (*oninput)(InputField *, int);
};

typedef struct
{
    int offset;
    size_t count;
    InputField *arr;
} InputFields;

InputField create_input_field(String str);
void append_input_field(InputField i);
void DrawInputField(InputField i);
void DrawInputFields(void);
int inside_rect(Point p, Rect r);

#endif // PM_UI_H_
