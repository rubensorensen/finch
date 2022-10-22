#include "finch/application/application.h"
#include "finch/core/core.h"
#include "finch/log/log.h"
#include "finch/utils/string.h"
#include "finch/platform/platform.h"

#include <math.h>
#include <stdlib.h>

typedef struct _ApplicationData {
    f32 time_elapsed_seconds;
    f32 horizontal_offset;
    f32 vertical_offset;
    f32 velocity;
} ApplicationData;

static ApplicationData* app_data;

static void handle_events(ApplicationState* application_state)
{
    while (application_state->unhandled_events) {
        FcEvent e = application_state->events[--application_state->unhandled_events];
        switch (e.type) {                
            case FC_EVENT_TYPE_WHEEL_SCROLLED: {
                if (e.scroll_wheel_vertical_direction != 0) {
                    app_data->vertical_offset -= app_data->velocity * e.scroll_wheel_vertical_direction;
                    FC_INFO("Mouse wheel scrolled %s",
                           e.scroll_wheel_vertical_direction > 0 ? "up" : "down");
                }
                if (e.scroll_wheel_horizontal_direction != 0) {
                    app_data->horizontal_offset -= app_data->velocity * -e.scroll_wheel_horizontal_direction;
                    FC_INFO("Mouse wheel scrolled %s",
                           e.scroll_wheel_horizontal_direction > 0 ? "right" : "left");
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
    handle_events(application_state);
    
    InputState* input_state = &application_state->input_state;
    
    app_data->velocity = input_state->key_is_down[FC_KEY_SPACE] ? 500.0f : 250.0f;
    if (input_state->key_is_down[FC_KEY_W]) {
        app_data->vertical_offset += app_data->velocity * dt;
    }
    if (input_state->key_is_down[FC_KEY_S]) {
        app_data->vertical_offset -= app_data->velocity * dt;
    }
    if (input_state->key_is_down[FC_KEY_A]) {
        app_data->horizontal_offset += app_data->velocity * dt;
    }
    if (input_state->key_is_down[FC_KEY_D]) {
        app_data->horizontal_offset -= app_data->velocity * dt;
    }
    if (input_state->button_is_down[FC_BUTTON_LEFT]) {
        FC_INFO("dx: %d, dy: %d", input_state->mouse_dx, input_state->mouse_dy);
        app_data->horizontal_offset += input_state->mouse_dx;
        app_data->vertical_offset   += input_state->mouse_dy;
    }
    
    app_data->time_elapsed_seconds += dt;
}

void fc_application_deinit(ApplicationState* application_state)
{
    (void)application_state;

    free(app_data);
}
