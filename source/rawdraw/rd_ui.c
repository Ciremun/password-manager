// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "rawdraw/vendor/rawdraw_sf.h"

#include "rawdraw/rd_ui.h"

InputFields input_fields;

int inside_rect(Point p, Rect r)
{
    return p.x >= r.p1.x && p.y >= r.p1.y && p.x <= r.p2.x && p.y <= r.p2.y;
}

void add_input_field(InputField i)
{
    input_fields.arr[input_fields.count++] = i;
}

void DrawInputField(InputField i)
{
    static const int text_height = 10;
    int rect_height = i.rect.p2.y - i.rect.p1.y;
    CNFGPenX = i.rect.p1.x + 10;
    CNFGPenY = i.rect.p1.y + rect_height / 2 - text_height;
    if (i.focused)
        CNFGColor(COLOR(0xFF0000FF));
    else
        CNFGColor(i.rect.color);
    CNFGTackRectangle(i.rect.p1.x, i.rect.p1.y, i.rect.p2.x, i.rect.p2.y);
    CNFGColor(i.text.color);
    CNFGDrawText((char *)i.text.string.data, i.text.font_size);
}

void DrawInputFields()
{
    for (size_t i = 0; i < input_fields.count; ++i)
        DrawInputField(input_fields.arr[i]);
}
