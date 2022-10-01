#include "core.h"
#include "game.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static f32 time_elapsed_seconds;
static f32 horizontal_offset;
static f32 vertical_offset;

static const char* keys[] = {
    "none", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "3", "4", "5", "6", "7", "8", "9", "left_ctrl", "right_ctrl", "left_shift", "right_shift", "left_alt", "right_alt", "space", "tab", "esc", "enter", "backspace"
};

static const char* buttons[] = {
    "None", "Left", "Right", "Middle"
};

static b32 key_is_down[FC_KEY_COUNT];

void game_update(GameState* game_state, f32 dt)
{
    // Handling events
    while (game_state->unhandled_events) {
        FcEvent e = game_state->events[--game_state->unhandled_events];
        switch (e.type) {
            case FC_EVENT_TYPE_NONE: {
                fprintf(stderr, "[ERROR] Game received event of type None\n");
                exit(EXIT_FAILURE);
            }
                
            case FC_EVENT_TYPE_BUTTON_PRESSED: {
                printf("[EVENT] %s mouse button pressed at (%u, %u)\n",
                       buttons[e.button], e.mouse_x, e.mouse_y);
            } break;
                
            case FC_EVENT_TYPE_BUTTON_RELEASED: {
                printf("[EVENT] %s mouse button released at (%u, %u)\n",
                       buttons[e.button], e.mouse_x, e.mouse_y);
            } break;
                
            case FC_EVENT_TYPE_KEY_PRESSED: {
                printf("[EVENT] Key pressed: %s\n", keys[e.key]);
                key_is_down[e.key] = true;
            } break;
            case FC_EVENT_TYPE_KEY_RELEASED: {
                printf("[EVENT] Key released: %s\n", keys[e.key]);
                key_is_down[e.key] = false;
                
            } break;
            case FC_EVENT_TYPE_WHEEL_SCROLLED: {
                printf("[EVENT] Mouse wheel scrolled %s\n",
                       e.scroll_wheel_direction > 0 ? "up" : "down");
            } break;
            case FC_EVENT_TYPE_MOUSE_MOVED: {
                /* printf("[EVENT] Mouse moved to (%u, %u)\n", e.mouse_x, e.mouse_y); */
            } break;
            default: {}
        }
    }

    f32 velocity = 2.5f;
    if (key_is_down[FC_KEY_SPACE]) {
        velocity *= 2.0f;
    }
    
    if (key_is_down[FC_KEY_W]) {
        vertical_offset -= velocity;
    }
    if (key_is_down[FC_KEY_S]) {
        vertical_offset += velocity;
    }
    if (key_is_down[FC_KEY_A]) {
        horizontal_offset -= velocity;
    }
    if (key_is_down[FC_KEY_D]) {
        horizontal_offset += velocity;
    }
    
    // Rendering
    for (u32 j = 0; j < game_state->height_px; ++j) {
        for (u32 i = 0; i < game_state->width_px; ++i) {
            f32 u = i / (f32)game_state->width_px;
            f32 v = j / (f32)game_state->height_px;

            Color col = {0};
            col.r = (sinf(powf(u, v) * time_elapsed_seconds * 2.5f) + 1) / 2.0f * 255.0f;
            /* col.r = 0xFFu; */
            col.g = (u8)i + horizontal_offset;
            col.b = (u8)j + vertical_offset;
            col.a = 0xFFu;            

            u32 color = format_color(col);
            game_state->pixelbuffer[i + j * game_state->width_px] = color;
        }
    }
    
    time_elapsed_seconds += dt;
}
