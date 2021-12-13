#ifndef PM_UI_H_
#define PM_UI_H_

#include <stddef.h>
#include <stdint.h>

#include "pm_util.h"
#include "pm_rd_util.h"

typedef struct InputField InputField;

typedef struct
{
    String string;
    uint32_t color;
    int font_size;
    int offset;
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
    InputField *arr;
    size_t count;
} InputFields;

void add_input_field(InputField i);
void DrawInputField(InputField i);
void DrawInputFields();
int inside_rect(Point p, Rect r);

#endif // PM_UI_H_
