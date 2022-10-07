#include "finch/application/application.h"
#include "finch/core/core.h"
#include "finch/log/log.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static f32 time_elapsed_seconds;
static f32 horizontal_offset;
static f32 vertical_offset;
static f32 velocity;

static const char* keys[] = {
    "none", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "0", "1", "3", "4", "5", "6", "7", "8", "9", "left_ctrl", "right_ctrl", "left_shift", "right_shift", "left_alt", "right_alt", "space", "tab", "esc", "enter", "backspace", "caps_lock", "up", "down", "left", "right", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12"
};

static const char* buttons[] = {
    "None", "Left", "Right", "Middle"
};

extern int main(void);

static void handle_game_events(ApplicationState* application_state)
{
    while (application_state->unhandled_events) {
        FcEvent e = application_state->events[--application_state->unhandled_events];
        switch (e.type) {
            case FC_EVENT_TYPE_NONE: {
                FC_FATAL("Game received event of type %CrNone%Cn");
            } break;
                
            case FC_EVENT_TYPE_BUTTON_PRESSED: {
                FC_INFO("%Cb%Vs%Cn mouse button pressed at %Cp(%Vu, %Vu)%Cn",
                        buttons[e.button], e.mouse_x, e.mouse_y);
            } break;
                
            case FC_EVENT_TYPE_BUTTON_RELEASED: {
                FC_INFO("%Cb%Vs%Cn mouse button released at %Cp(%Vu, %Vu)%Cn",
                        buttons[e.button], e.mouse_x, e.mouse_y);
            } break;
                
            case FC_EVENT_TYPE_KEY_PRESSED: {
                FC_INFO("Key pressed: %Cb%Vs%Cn", keys[e.key]);
            } break;
                
            case FC_EVENT_TYPE_KEY_RELEASED: {
                FC_INFO("Key released: %Cb%Vs%Cn", keys[e.key]);
            } break;
                
            case FC_EVENT_TYPE_WHEEL_SCROLLED: {
                if (e.scroll_wheel_vertical_direction != 0) {
                    vertical_offset -= velocity * e.scroll_wheel_vertical_direction;
                    FC_INFO("Mouse wheel scrolled %Cb%Vs%Cn",
                           e.scroll_wheel_vertical_direction > 0 ? "up" : "down");
                }
                if (e.scroll_wheel_horizontal_direction != 0) {
                    horizontal_offset -= velocity * -e.scroll_wheel_horizontal_direction;
                    FC_INFO("Mouse wheel scrolled %Cb%Vs%Cn",
                           e.scroll_wheel_horizontal_direction > 0 ? "right" : "left");
                }
            } break;
            case FC_EVENT_TYPE_MOUSE_MOVED: {
            } break;
            default: {}
        }
    }

}

static u32 format_color(Color col)
{
    return (u32)(col.a << 24 |
                 col.r << 16 |
                 col.g << 8  |
                 col.b << 0);
}

void fc_application_init(ApplicationState* application_state)
{
    application_state->name = "Sandbox";
    application_state->width_px = 1280;
    application_state->height_px = 720;
}

void fc_application_update(ApplicationState* application_state, f64 dt)
{   
    handle_game_events(application_state);
    InputState* input_state = &application_state->input_state;
    
    velocity = input_state->key_is_down[FC_KEY_SPACE] ? 500.0f : 250.0f;
    
    if (input_state->key_is_down[FC_KEY_W]) {
        vertical_offset += velocity * dt;
    }
    if (input_state->key_is_down[FC_KEY_S]) {
        vertical_offset -= velocity * dt;
    }
    if (input_state->key_is_down[FC_KEY_A]) {
        horizontal_offset += velocity * dt;
    }
    if (input_state->key_is_down[FC_KEY_D]) {
        horizontal_offset -= velocity * dt;
    }
    if (input_state->button_is_down[FC_BUTTON_LEFT]) {
        FC_INFO("dx: %Vd, \tdy: %Vd",
                input_state->mouse_dx, input_state->mouse_dy);
        horizontal_offset += input_state->mouse_dx;
        vertical_offset   += input_state->mouse_dy;
    }
    
    // Rendering
    for (u32 j = 0; j < application_state->height_px; ++j) {
        for (u32 i = 0; i < application_state->width_px; ++i) {
            f32 u = i / (f32)application_state->width_px;
            f32 v = j / (f32)application_state->height_px;

            Color col = {0};
            col.r = (sinf(powf(u, v) * time_elapsed_seconds * 2.5f) + 1) / 2.0f * 255.0f;
            /* col.r = 0xFFu; */
            /* col.r = 0x00u; */
            col.b = (u8)i - horizontal_offset;
            col.g = (u8)j - vertical_offset;
            col.a = 0xFFu;            

            application_state->pixelbuffer[i + j * application_state->width_px] = format_color(col);
        }
    }
    
    time_elapsed_seconds += dt;
}

void fc_application_deinit(ApplicationState* application_state)
{
    (void)application_state;
}
