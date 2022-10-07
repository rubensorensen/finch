#include "core.h"
#include "game.h"
#include "log/log.h"

#include <stdio.h>
#include <stdlib.h>

// Implemented in application
extern void game_update(GameState*, f64);

// Implemented in platform layer
extern void platform_init(GameState*);
extern void platform_deinit(GameState*);
extern void platform_poll_events(GameState*);
extern void platform_put_pixelbuffer_on_screen(GameState*);
extern f64 platform_get_epoch_time();
extern WindowAttributes* platform_get_window_attributes();
extern void platform_set_window_title(const char*);

int main(void)
{
    fc_logger_init(fc_get_engine_logger(), "%TT (%Fn:%Fl) %CcFINCH%Cn [%Gl] ");
    fc_logger_init(fc_get_application_logger(), "%TT (%Fn:%Fl) %CcAPPLICATION%Cn [%Gl] ");

    GameState game_state = {0};
    platform_init(&game_state);
    
    f64 prev_time = platform_get_epoch_time();

    f64 time_since_window_title_updated = 0.0;
    
    game_state.running = true;
    while (game_state.running) {
        f64 curr_time = platform_get_epoch_time();
        f64 delta_time = curr_time - prev_time;
        
        platform_poll_events(&game_state);
        game_update(&game_state, delta_time);
        platform_put_pixelbuffer_on_screen(&game_state);

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

    platform_deinit(&game_state);
    
    return EXIT_SUCCESS;
}

