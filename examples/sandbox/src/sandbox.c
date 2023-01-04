#include "finch/application.h"
#include "finch/core.h"
#include "finch/log.h"
#include "finch/utils.h"
#include "finch/platform.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static b32 left_mouse_button_down;

typedef struct _ApplicationData {
    f32 time_elapsed_seconds;
    f32 horizontal_offset;
    f32 vertical_offset;
    f32 velocity;

    int cursor_style;
} ApplicationData;

static ApplicationData* app_data;

f32 
get_horizontal_offset(void)
{
    return app_data->horizontal_offset;
}

f32 
get_vertical_offset(void)
{
    return app_data->vertical_offset;
}

static void handle_events(ApplicationState* application_state, f64 dt)
{
    while (application_state->unhandled_events) {
        FcEvent e = application_state->events[--application_state->unhandled_events];
        switch (e.type) {
            case FC_EVENT_TYPE_MOUSE_MOVED: {
                if (left_mouse_button_down) {
                    app_data->horizontal_offset += (f64)e.mouse_dx * dt;
                    app_data->vertical_offset += (f64)e.mouse_dy * dt;
                }
            } break;
            case FC_EVENT_TYPE_WHEEL_SCROLLED: {
                if (e.scroll_wheel_vertical_direction != 0) {

                }
                if (e.scroll_wheel_horizontal_direction != 0) {

                }
            } break;
            case FC_EVENT_TYPE_BUTTON_PRESSED: {
                if (e.button == FC_BUTTON_LEFT) {
                    left_mouse_button_down = FC_TRUE;
                }
            } break;
            case FC_EVENT_TYPE_BUTTON_RELEASED: {
                if (e.button == FC_BUTTON_LEFT) {
                    left_mouse_button_down = FC_FALSE;
                }
            } break;
            case FC_EVENT_TYPE_KEY_PRESSED: {
                if (e.key == FC_KEY_ESC || e.key == FC_KEY_Q) {
                    application_state->running = false;
                }
                else if (e.key == FC_KEY_TAB) {
                    app_data->cursor_style = (app_data->cursor_style + 1) % FC_CURSOR_STYLE_COUNT;
                    platform_set_cursor_style((FcCursorStyle)app_data->cursor_style);
                } else if (e.key == FC_KEY_SPACE) {
                    platform_move_cursor(application_state->width_px / 2,
                                         application_state->height_px / 2);
                }
                else if (e.key == FC_KEY_ENTER) {
                    platform_toggle_fullscreen();
                }
                else if(e.key == FC_KEY_C) {
                    platform_toggle_cursor_confinement();
                }
                else if (e.key == FC_KEY_V) {
                    platform_toggle_cursor_centered();
                }
            } break;
            default: {}
        }
    }
}

void fc_application_init(ApplicationState* application_state)
{
    app_data = (ApplicationData*)malloc(sizeof(ApplicationData));
    app_data->time_elapsed_seconds = 0.0f;
    app_data->horizontal_offset = 0.0f;
    app_data->vertical_offset = 0.0f;
    app_data->velocity = 0.0f;

    application_state->name = "Sandbox";
    application_state->width_px = 1280;
    application_state->height_px = 720;
}

void fc_application_update(ApplicationState* application_state, f64 dt)
{
    handle_events(application_state, dt);

    app_data->time_elapsed_seconds += dt;
}

void fc_application_deinit(ApplicationState* application_state)
{
    UNUSED(application_state);

    free(app_data);
}
