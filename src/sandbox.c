#include "core.h"

#include <stdio.h>
#include <math.h>

static f32 offset;

void game_update(GameState* game_state, f32 dt)
{
    for (u32 j = 0; j < game_state->height_px; ++j) {
        for (u32 i = 0; i < game_state->width_px; ++i) {
            f32 u = i / (f32)game_state->width_px;
            f32 v = j / (f32)game_state->height_px;

            Color col = {0};
            col.r = (sinf(u * v * offset * 2.5f) + 1) / 2.0f * 255.0f;
            col.g = (u8)i + offset * 125;
            col.b = (u8)j + offset * 60;
            col.a = 0xFFu;            

            u32 color = format_color(col);
            game_state->pixelbuffer[i + j * game_state->width_px] = color;
        }
    }
    offset += dt;
}
