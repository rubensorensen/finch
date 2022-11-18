#include "finch/core.h"
#include "finch/application.h"
#include "finch/log.h"
#include "finch/platform.h"
#include "finch/math.h"

#include <stdio.h>
#include <stdlib.h>

extern void linux_draw_frame(f64);
extern void vulkan_wait_for_device_idle(void);
extern void vulkan_init(ApplicationState* app_state);
extern void vulkan_deinit(void);

int main(void)
{
    ApplicationState application_state = {0};
    fc_application_init(&application_state);
    platform_init(&application_state);
    vulkan_init(&application_state);

    f64 prev_time = platform_get_epoch_time();

    f64 time_since_window_title_updated = 0.0;

    application_state.running = true;
    while (application_state.running) {
        f64 curr_time = platform_get_epoch_time();
        f64 delta_time = curr_time - prev_time;

        platform_poll_events(&application_state);
        fc_application_update(&application_state, delta_time);
        linux_draw_frame(delta_time);

        // Update fps in window title approx. every second
        time_since_window_title_updated += delta_time;
        if (time_since_window_title_updated > 1.0) {
            char buf[1000];
            sprintf(buf, "%s - %dfps",
                    platform_get_window_attributes()->title, (u32)(1.0 / delta_time));
            platform_set_window_title(buf);
            time_since_window_title_updated = 0.0f;
        }

        prev_time = curr_time;
    }

    vulkan_wait_for_device_idle();

    vulkan_deinit();
    fc_application_deinit(&application_state);
    platform_deinit(&application_state);

    return EXIT_SUCCESS;
}
