#include "finch/core/core.h"
#include "finch/application/application.h"
#include "finch/log/log.h"

void fc_application_init(ApplicationState* app_state)
{
    app_state->name = "Minimal Application";
    app_state->width_px = 800;
    app_state->height_px = 600;

    FC_INFO("Application '%Vs' is initialized!", app_state->name);
}

void fc_application_update(ApplicationState* app_state, f64 delta_time)
{
    (void)delta_time; // Unused variable - suppresses warning
    
    // Clear to red
    u32* pixel = app_state->pixelbuffer;
    for (u32 y = 0; y < app_state->height_px; ++y) {
        for (u32 x = 0; x < app_state->width_px; ++x) {
            *pixel++ = 0xFFFF0000u; // Color scheme: AARRGGBB
        }
    }
}

void fc_application_deinit(ApplicationState* app_state)
{
    FC_INFO("Application '%Vs' is deinitializing", app_state->name);
}
