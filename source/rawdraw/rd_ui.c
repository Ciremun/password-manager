// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "rawdraw/vendor/rawdraw_sf.h"

#include "core/pm_util.h"
#include "rawdraw/rd_util.h"
#include "rawdraw/rd_ui.h"
#include "rawdraw/rd_event.h"

InputFields input_fields = {0};

extern short w;
extern short h;
extern RDWindow window;

int inside_rect(Point p, Rect r)
{
    return p.x >= r.p1.x && p.y >= r.p1.y && p.x <= r.p2.x && p.y <= r.p2.y;
}

InputField create_input_field(String str)
{
    Point input_field_dim = { .x = w, .y = RD_INPUT_FIELD_HEIGHT };
    return (InputField){
        .rect = (Rect){
            .color = WHITE,
            .p1 = (Point){
                .x = 0,
                .y = 0,
            },
            .p2 = input_field_dim,
        },
        .text = (Text){
            .string = (String){
                .data = str.data,
                .length = str.length,
            },
            .color = BLACK,
            .font_size = RD_INPUT_FIELD_FONT_SIZE,
        },
        .focused = 0,
        .oninput = oninput,
    };
}

void append_input_field(InputField i)
{
    int y_offset = input_fields.count ? input_fields.arr[input_fields.count - 1].rect.p2.y : 0;
    input_fields.offset = y_offset + RD_INPUT_FIELD_MARGIN;
    i.rect.p1.y += input_fields.offset;
    i.rect.p2.y += input_fields.offset;
    input_fields.arr[input_fields.count++] = i;
}

void DrawInputField(InputField i)
{
    static const int text_height = 10;
    const int rect_height = i.rect.p2.y - i.rect.p1.y;
    CNFGPenX = i.rect.p1.x + 10;
    CNFGPenY = i.rect.p1.y + rect_height / 2 - text_height;
    if (i.focused)
        CNFGColor(GRAY);
    else
        CNFGColor(i.rect.color);
    static const int32_t radius = 24;
    const int32_t centreX = i.rect.p1.x + radius;
    const int32_t centreY = i.rect.p1.y + radius - RD_INPUT_FIELD_MARGIN;
    const int32_t diameter = (radius * 2);
    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);
    while (x >= y)
    {
        CNFGTackSegment(centreX - x, centreY + y, centreX + x + i.rect.p2.x - radius * 2, centreY + y);
        CNFGTackSegment(centreX - x, centreY - y, centreX + x + i.rect.p2.x - radius * 2, centreY - y);
        if (error <= 0)
        {
         ++y;
         error += ty;
         ty += 2;
        }
        else if (error > 0)
        {
         --x;
         tx += 2;
         error += (tx - diameter);
        }
    }
    CNFGColor(i.text.color);
    CNFGDrawText((char *)i.text.string.data, i.text.font_size);
}

void DrawInputFields(void)
{
    for (size_t i = window.scroll; i < input_fields.count; ++i)
    {
        if (input_fields.arr[i].rect.p2.y >= h)
            break;
        DrawInputField(input_fields.arr[i]);
    }
}
