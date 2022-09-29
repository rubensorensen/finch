#include "core.h"

u32 format_color(Color col)
{
    return (u32)(col.a << 24 |
                 col.r << 16 |
                 col.g << 8  |
                 col.b << 0);
}
