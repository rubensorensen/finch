#ifndef _FINCH_PLATFORM_H
#define _FINCH_PLATFORM_H

#include "finch/application.h"

// Implemented in platform layer
void platform_init(ApplicationState*);
void platform_deinit(ApplicationState*);
void platform_poll_events(ApplicationState*);
void platform_put_pixelbuffer_on_screen(ApplicationState*);
f64 platform_get_epoch_time();
WindowAttributes* platform_get_window_attributes();
void platform_set_window_title(const char*);
b32 platform_terminal_supports_colors();
b32 platform_stdout_is_terminal();
b32 platform_stderr_is_terminal();
void platform_set_terminal_color(FcTerminalColor);
void platform_get_framebuffer_size(u32* width, u32* height);
void platform_show_cursor();
void platform_hide_cursor();
void platform_move_cursor(u32 x, u32 y);
void platform_toggle_fullscreen();
void platform_toggle_cursor_centered();
void platform_toggle_cursor_confinement();

#endif // _FINCH_PLATFORM_H
