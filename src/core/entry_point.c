#include "finch/core/core.h"
#include "finch/application/application.h"
#include "finch/log/log.h"
#include "finch/platform/platform.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    ApplicationState application_state = {0};
    fc_application_init(&application_state);
    platform_init(&application_state);
    
    f64 prev_time = platform_get_epoch_time();

    f64 time_since_window_title_updated = 0.0;
    
    application_state.running = true;
    while (application_state.running) {
        f64 curr_time = platform_get_epoch_time();
        f64 delta_time = curr_time - prev_time;
        
        platform_poll_events(&application_state);
        fc_application_update(&application_state, delta_time);
        platform_put_pixelbuffer_on_screen(&application_state);

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

    platform_deinit(&application_state);
    fc_application_deinit(&application_state);
    
    return EXIT_SUCCESS;
}
