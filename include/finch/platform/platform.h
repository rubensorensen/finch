#ifndef FINCH_PLATFORM_PLATFORM_H
#define FINCH_PLATFORM_PLATFORM_H

#include "finch/application/application.h"
#include "finch/log/log.h"

// Implemented in platform layer
void platform_init(ApplicationState*);
void platform_deinit(ApplicationState*);
void platform_poll_events(ApplicationState*);
void platform_put_pixelbuffer_on_screen(ApplicationState*);
f64 platform_get_epoch_time();
WindowAttributes* platform_get_window_attributes();
void platform_set_window_title(const char*);
void platform_write_to_stdout(char*);
void platform_write_to_stderr(char*);
b32 platform_terminal_supports_colors();
b32 platform_stdout_is_terminal();
b32 platform_stderr_is_terminal();
void platform_set_terminal_color(FcTerminalColor);

#endif // FINCH_PLATFORM_PLATFORM_H
