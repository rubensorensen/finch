#include "core.h"
#include "game.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static f32 time_elapsed_seconds;
static f32 horizontal_offset;
static f32 vertical_offset;
static f32 velocity;

static const char* keys[] = {
    "none", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "3", "4", "5", "6", "7", "8", "9", "left_ctrl", "right_ctrl", "left_shift", "right_shift", "left_alt", "right_alt", "space", "tab", "esc", "enter", "backspace"
};

static const char* buttons[] = {
    "None", "Left", "Right", "Middle"
};

static b32 key_is_down[FC_KEY_COUNT];
static b32 button_is_down[FC_BUTTON_COUNT];
static u32 mouse_x, mouse_y;
static u32 mouse_x_prev, mouse_y_prev;
static b32 button_was_down[FC_BUTTON_COUNT];

static void handle_events(GameState* game_state)
{
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
                button_is_down[e.button] = true;
                button_was_down[e.button] = false;
            } break;
                
            case FC_EVENT_TYPE_BUTTON_RELEASED: {
                printf("[EVENT] %s mouse button released at (%u, %u)\n",
                       buttons[e.button], e.mouse_x, e.mouse_y);
                button_is_down[e.button] = false;
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
                if (e.scroll_wheel_vertical_direction != 0) {
                    vertical_offset += velocity * -e.scroll_wheel_vertical_direction;
                    printf("[EVENT] Mouse wheel scrolled %s\n",
                           e.scroll_wheel_vertical_direction > 0 ? "up" : "down");
                }
                if (e.scroll_wheel_horizontal_direction != 0) {
                    horizontal_offset += velocity * e.scroll_wheel_horizontal_direction;
                    printf("[EVENT] Mouse wheel scrolled %s\n",
                           e.scroll_wheel_horizontal_direction > 0 ? "right" : "left");
                }
            } break;
            case FC_EVENT_TYPE_MOUSE_MOVED: {
                mouse_x_prev = mouse_x;
                mouse_y_prev = mouse_y;
                mouse_x = e.mouse_x;
                mouse_y = e.mouse_y;
            } break;
            default: {}
        }
    }

}

void game_update(GameState* game_state, f32 dt)
{   
    handle_events(game_state);
    
    velocity = key_is_down[FC_KEY_SPACE] ? 5.0f : 2.5f;
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
    if (button_is_down[FC_BUTTON_LEFT]) {
        if (!button_was_down[FC_BUTTON_LEFT]) {
            button_was_down[FC_BUTTON_LEFT] = true;
        } else {
            s32 dx = mouse_x - mouse_x_prev;
            s32 dy = mouse_y - mouse_y_prev;

            horizontal_offset += dx;
            vertical_offset += dy;
        }
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
