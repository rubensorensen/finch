#define _POSIX_C_SOURCE 199309L

#include "finch/core.h"
#include "finch/utils.h"
#include "finch/log.h"
#include "finch/events.h"
#include "finch/application.h"

#include "finch/linux_finch.h"
#include "finch/vulkan.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

static X11State x11_state;

static u32 prev_x, prev_y;

// To be used by modules requiring access to x11 state
X11State* x11_get_ptr_to_state(void)
{
    return &x11_state;
}

extern void vulkan_draw_frame(f64 dt);

void linux_draw_frame(f64 dt)
{
    vulkan_draw_frame(dt);
}

static s32 terminal_supports_colors = -1;

static void
x11_set_cursor_style(Display* display, Window window, FcCursorStyle style)
{
    Cursor cursor;
    switch (style) {
        case FC_CURSOR_STYLE_HIDDEN: {
            XColor color = {0};
            const char data[] = {0};
            Pixmap pixmap = XCreateBitmapFromData(display, window, data, 1, 1);
            cursor = XCreatePixmapCursor(display, pixmap, pixmap, &color, &color, 0, 0);
            XFreePixmap(display, pixmap);
        } break;
        case FC_CURSOR_STYLE_DEFAULT: {
            cursor = XCreateFontCursor(display, XC_arrow);
        } break;
        case FC_CURSOR_STYLE_POINTER: {
            cursor = XCreateFontCursor(display, XC_hand2);
        } break;
        case FC_CURSOR_STYLE_WAIT: {
            cursor = XCreateFontCursor(display, XC_watch);
        } break;
        case FC_CURSOR_STYLE_TEXT: {
            cursor = XCreateFontCursor(display, XC_xterm);
        } break;
        case FC_CURSOR_STYLE_CROSSHAIR: {
            cursor = XCreateFontCursor(display, XC_crosshair);
        } break;
        case FC_CURSOR_STYLE_MOVE: {
            cursor = XCreateFontCursor(display, XC_fleur);
        } break;
        case FC_CURSOR_STYLE_NOT_ALLOWED: {
            cursor = XCreateFontCursor(display, XC_pirate);
        } break;
        case FC_CURSOR_STYLE_UP_ARROW: {
            cursor = XCreateFontCursor(display, XC_sb_up_arrow);
        } break;
        case FC_CURSOR_STYLE_DOWN_ARROW: {
            cursor = XCreateFontCursor(display, XC_sb_down_arrow);
        } break;
        case FC_CURSOR_STYLE_LEFT_ARROW: {
            cursor = XCreateFontCursor(display, XC_sb_left_arrow);
        } break;
        case FC_CURSOR_STYLE_RIGHT_ARROW: {
            cursor = XCreateFontCursor(display, XC_sb_right_arrow);
        } break;
        case FC_CURSOR_STYLE_UP_DOWN_ARROW: {
            cursor = XCreateFontCursor(display, XC_sb_v_double_arrow);
        } break;
        case FC_CURSOR_STYLE_LEFT_RIGHT_ARROW: {
            cursor = XCreateFontCursor(display, XC_sb_h_double_arrow);
        } break;
        default: {
            FC_WARN("Unknown finch cursor style '%d', falling back to default",
                    style);
            cursor = XCreateFontCursor(display, XC_arrow);
        }
    }
    
    XDefineCursor(display, window, cursor);
    XFreeCursor(display, cursor);
}

static void
x11_move_cursor(Display* display, Window window, u32 x, u32 y)
{
    XWarpPointer(display, None, window, 0, 0, 0, 0, x, y);
}

static void
x11_toggle_fullscreen(Display* display, Window window)
{
    Atom wm_state = XInternAtom (display, "_NET_WM_STATE", true );
    Atom wm_fullscreen = XInternAtom (display, "_NET_WM_STATE_FULLSCREEN", true );

    XChangeProperty(display, window, wm_state, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *)&wm_fullscreen, 1);
    XEvent event = {0};
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = wm_state;
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2;
    event.xclient.data.l[1] = wm_fullscreen;
    event.xclient.data.l[2] = 0;
    event.xclient.data.l[3] = 0;
    event.xclient.data.l[4] = 0;
    XSendEvent(display, XDefaultRootWindow(display), False,
               StructureNotifyMask | ResizeRedirectMask, &event);
}

static void
x11_toggle_cursor_confinement(void)
{
    x11_state.cursor_confined = !x11_state.cursor_confined;
}

static void
x11_toggle_cursor_centered(void)
{
    x11_state.cursor_centered = !x11_state.cursor_centered;
}

static void
x11_init(X11State* x11_state)
{
    x11_state->display = XOpenDisplay(NULL);
    if (x11_state->display == NULL) {
        FC_ENGINE_ERROR("Could not open default display.");
        exit(EXIT_FAILURE);
    }

    x11_state->screen = DefaultScreen(x11_state->display);
    
    XVisualInfo visual_info;
    XMatchVisualInfo(x11_state->display, x11_state->screen, 32, TrueColor, &visual_info);
    
    XSetWindowAttributes win_attribs = {0};
    win_attribs.colormap = XCreateColormap(x11_state->display,
                                           DefaultRootWindow(x11_state->display),
                                           visual_info.visual, AllocNone);
    win_attribs.background_pixmap = BlackPixel(x11_state->display, x11_state->screen);
    win_attribs.background_pixel = BlackPixel(x11_state->display, x11_state->screen);
    win_attribs.border_pixel = 0;
    win_attribs.bit_gravity = NorthWestGravity;
    win_attribs.backing_store = WhenMapped;
    win_attribs.event_mask = (KeyPressMask | KeyReleaseMask | ButtonPressMask |
                              ButtonReleaseMask | PointerMotionMask |
                              StructureNotifyMask | ExposureMask);

    x11_state->window = XCreateWindow(x11_state->display,
                                      XRootWindow(x11_state->display, x11_state->screen),
                                      0, 0,
                                      x11_state->window_attributes.width,
                                      x11_state->window_attributes.height,
                                      0, visual_info.depth, InputOutput,
                                      visual_info.visual,
                                      CWColormap | CWBackPixmap | CWBorderPixel |
                                      CWBackPixel | CWBitGravity | CWBackingStore |
                                      CWEventMask,
                                      &win_attribs);
    x11_state->gc = XCreateGC(x11_state->display, x11_state->window, 0, NULL);

    x11_state->wm_delete_window = XInternAtom(x11_state->display,
                                              "WM_DELETE_WINDOW", False);
    if (x11_state->wm_delete_window == None) {
        FC_WARN("Failed to get x11 atom with name WM_DELETE_WINDOW");
    }
    
    XSetWMProtocols(x11_state->display, x11_state->window,
                    &x11_state->wm_delete_window, 1);
    
    
    // Set window titlebar name
    XStoreName(x11_state->display, x11_state->window,
               x11_state->window_attributes.title);
    
    // Set window (Not titlebar) name and class
    XClassHint* class_hint = XAllocClassHint();
    if (class_hint)
    {
        class_hint->res_name = x11_state->window_attributes.title;
        class_hint->res_class = "Finch";
        XSetClassHint(x11_state->display, x11_state->window, class_hint);
        XFree(class_hint);
    }    
    
    /* x11_fullscreen(x11_state->display, x11_state->window); */
    XMapWindow(x11_state->display, x11_state->window);

    x11_set_cursor_style(x11_state->display, x11_state->window,
                         FC_CURSOR_STYLE_DEFAULT);
}

static void
x11_deinit(X11State* x11_state)
{
    XUnmapWindow(x11_state->display, x11_state->window);
    XFreeGC(x11_state->display, x11_state->gc);
    XDestroyWindow(x11_state->display, x11_state->window);

    // Todo: I feel like I should be calling XCloseDisplay, but this leads
    // to a segfault when running on X11 (Not Wayland). So for now I just
    // unmap and destroy the window, and let the windowing system handle
    // the display closing.
    /* XCloseDisplay(x11_state->display); */
}

void
x11_get_framebuffer_size(X11State* x11_state, u32* width, u32* height)
{

    XWindowAttributes window_attributes;
    XGetWindowAttributes(x11_state->display, x11_state->window, &window_attributes);

    if (width) {
        *width = window_attributes.width;
    }

    if (height) {
        *height = window_attributes.height;
    }
}

extern b32 vulkan_initialized(void);
extern void recreate_swap_chain(void);

static void
window_resize(ApplicationState* application_state, u32 new_width, u32 new_height)
{
    application_state->width_px  = new_width;
    application_state->height_px = new_height;
    if (vulkan_initialized()) recreate_swap_chain();
}

static void
x11_resize_window(X11State* x11_state, u32 new_width, u32 new_height)
{
    x11_state->window_attributes.width  = new_width;
    x11_state->window_attributes.height = new_height;
}

static void
x11_handle_events(X11State* x11_state, ApplicationState* application_state)
{
    application_state->unhandled_events = 0;
    
    b32 mouse_moved = FC_FALSE;
    u32 mouse_x, mouse_y;
    while (XPending(x11_state->display) > 0) {
        FcEvent finch_event = {0};

        XEvent e = {0};
        XAllowEvents(x11_state->display, AsyncPointer, CurrentTime);
        XNextEvent(x11_state->display, &e);

        switch (e.type) {
            case KeyPress: {
                finch_event.type = FC_EVENT_TYPE_KEY_PRESSED;
                KeySym key = XLookupKeysym(&e.xkey, 0);
                FcKey finch_key = FC_KEY_NONE;

                // Character key
                if (key >= 'a' && key <= 'z') {
                    finch_key = (FcKey)(key - 'a' + FC_KEY_A);
                }

                // Numeric key
                else if (key >= '0' && key <= '9') {
                    finch_key = (FcKey)(key - '0' + FC_KEY_0);
                }

                // Special keys
                else {
                    switch (key) {
                        case 32: {
                            finch_key = FC_KEY_SPACE;
                        } break;
                        case 65507: {
                            finch_key = FC_KEY_LEFT_CTRL;
                        } break;
                        case 65508: {
                            finch_key = FC_KEY_RIGHT_CTRL;
                        } break;
                        case 65505: {
                            finch_key = FC_KEY_LEFT_SHIFT;
                        } break;
                        case 65506: {
                            finch_key = FC_KEY_RIGHT_SHIFT;
                        } break;
                        case 65513: {
                            finch_key = FC_KEY_LEFT_ALT;
                        } break;
                        case 65027: {
                            finch_key = FC_KEY_RIGHT_ALT;
                        } break;
                        case 65289: {
                            finch_key = FC_KEY_TAB;
                        } break;
                        case 65307: {
                            finch_key = FC_KEY_ESC;
                        } break;
                        case 65293: {
                            finch_key = FC_KEY_ENTER;
                        } break;
                        case 65288: {
                            finch_key = FC_KEY_BACKSPACE;
                        } break;
                        case 65509: {
                            finch_key = FC_KEY_CAPS_LOCK;
                        } break;
                        case 65362: {
                            finch_key = FC_KEY_UP;
                        } break;
                        case 65364: {
                            finch_key = FC_KEY_DOWN;
                        } break;
                        case 65361: {
                            finch_key = FC_KEY_LEFT;
                        } break;
                        case 65363: {
                            finch_key = FC_KEY_RIGHT;
                        } break;
                        case 65470: {
                            finch_key = FC_KEY_F1;
                        } break;
                        case 65471: {
                            finch_key = FC_KEY_F2;
                        } break;
                        case 65472: {
                            finch_key = FC_KEY_F3;
                        } break;
                        case 65473: {
                            finch_key = FC_KEY_F4;
                        } break;
                        case 65474: {
                            finch_key = FC_KEY_F5;
                        } break;
                        case 65475: {
                            finch_key = FC_KEY_F6;
                        } break;
                        case 65476: {
                            finch_key = FC_KEY_F7;
                        } break;
                        case 65477: {
                            finch_key = FC_KEY_F8;
                        } break;
                        case 65478: {
                            finch_key = FC_KEY_F9;
                        } break;
                        case 65479: {
                            finch_key = FC_KEY_F10;
                        } break;
                        case 65480: {
                            finch_key = FC_KEY_F11;
                        } break;
                        case 65481: {
                            finch_key = FC_KEY_F12;
                        } break;
                        default: {
                            // Unhandled key
                            FC_ENGINE_WARN("Unhandled key press event (Key: %zu)", key);
                            finch_event.type = FC_EVENT_TYPE_NONE;
                            finch_key = FC_KEY_NONE;
                        }
                    }
                }

                finch_event.key = finch_key;

            } break;
            case KeyRelease: {

                // Remove auto-generated key-releases and key-presses
                // from repeats.
                if (XEventsQueued(x11_state->display, QueuedAfterReading)) {
                    XEvent ne;
                    XPeekEvent(x11_state->display, &ne);

                    if (ne.type == KeyPress && ne.xkey.time == e.xkey.time &&
                        ne.xkey.keycode == e.xkey.keycode) {
                        XNextEvent (x11_state->display, &e);
                        break;
                    }
                }

                finch_event.type = FC_EVENT_TYPE_KEY_RELEASED;
                KeySym key = XLookupKeysym(&e.xkey, 0);
                FcKey finch_key = FC_KEY_NONE;

                // Character key
                if (key >= 'a' && key <= 'z') {
                    finch_key = (FcKey)(key - 'a' + FC_KEY_A);
                }

                // Numeric key
                else if (key >= '0' && key <= '9') {
                    finch_key = (FcKey)(key - '0' + FC_KEY_0);
                }

                else {
                    // Special keys
                    switch (key) {
                        case 32: {
                            finch_key = FC_KEY_SPACE;
                        } break;
                        case 65507: {
                            finch_key = FC_KEY_LEFT_CTRL;
                        } break;
                        case 65508: {
                            finch_key = FC_KEY_RIGHT_CTRL;
                        } break;
                        case 65505: {
                            finch_key = FC_KEY_LEFT_SHIFT;
                        } break;
                        case 65506: {
                            finch_key = FC_KEY_RIGHT_SHIFT;
                        } break;
                        case 65513: {
                            finch_key = FC_KEY_LEFT_ALT;
                        } break;
                        case 65027: {
                            finch_key = FC_KEY_RIGHT_ALT;
                        } break;
                        case 65289: {
                            finch_key = FC_KEY_TAB;
                        } break;
                        case 65307: {
                            finch_key = FC_KEY_ESC;
                        } break;
                        case 65293: {
                            finch_key = FC_KEY_ENTER;
                        } break;
                        default: {
                            finch_event.type = FC_EVENT_TYPE_NONE;
                            finch_key = FC_KEY_NONE;
                        }
                    }
                }

                finch_event.key = finch_key;

            } break;
            case ButtonPress: {
                u32 button = e.xbutton.button;
                FcButton finch_button = FC_BUTTON_NONE;

                if (button >= 1 && button <= 3) {
                    finch_event.type = FC_EVENT_TYPE_BUTTON_PRESSED;
                }

                else if (button >= 4 && button <= 7) {
                    finch_event.type = FC_EVENT_TYPE_WHEEL_SCROLLED;
                }

                finch_event.mouse_x = e.xbutton.x;
                finch_event.mouse_y = e.xbutton.y;

                switch (button) {
                    case 1: {
                        // Left mouse button
                        finch_button = FC_BUTTON_LEFT;
                    } break;
                    case 2: {
                        // Middle mouse button
                        finch_button = FC_BUTTON_MIDDLE;
                    } break;
                    case 3: {
                        // Right mouse button
                        finch_button = FC_BUTTON_RIGHT;
                    } break;
                    case 4: {
                        // Mouse scroll up
                        finch_event.scroll_wheel_vertical_direction = 1;
                    } break;
                    case 5: {
                        // Mouse scroll down
                        finch_event.scroll_wheel_vertical_direction = -1;
                    } break;
                    case 6: {
                        // Mouse scroll left
                        finch_event.scroll_wheel_horizontal_direction = -1;
                    } break;
                    case 7: {
                        // Mouse scroll right
                        finch_event.scroll_wheel_horizontal_direction = 1;
                    } break;
                    default: {
                        // Unhandled button
                        finch_event.type = FC_EVENT_TYPE_NONE;
                        finch_button = FC_BUTTON_NONE;
                    }
                }

                finch_event.button = finch_button;

            } break;
            case ButtonRelease: {
                u32 button = e.xbutton.button;

                // Ignore release events from scrollwheels
                if (button < 1 || button > 3) {
                    break;
                }

                finch_event.type = FC_EVENT_TYPE_BUTTON_RELEASED;
                FcButton finch_button = FC_BUTTON_NONE;

                finch_event.mouse_x = e.xbutton.x;
                finch_event.mouse_y = e.xbutton.y;

                switch (button) {
                    case 1: {
                        // Left mouse button
                        finch_button = FC_BUTTON_LEFT;
                    } break;
                    case 2: {
                        // Middlbe mouse button
                        finch_button = FC_BUTTON_MIDDLE;
                    } break;
                    case 3: {
                        // Right mouse button
                        finch_button = FC_BUTTON_RIGHT;
                    } break;
                }

                finch_event.button = finch_button;

            } break;
            case MotionNotify: {
                mouse_moved = FC_TRUE;
                mouse_x = e.xmotion.x;
                mouse_y = e.xmotion.y;

//                finch_event.type = FC_EVENT_TYPE_MOUSE_MOVED;
//                finch_event.mouse_x = e.xmotion.x;
//                finch_event.mouse_y = e.xmotion.y;

            } break;
            case ClientMessage: {
                if ((Atom)e.xclient.data.l[0] == x11_state->wm_delete_window) {
                    application_state->running = false;
                }
            } break;
            case Expose: {
            } break;
            case ConfigureNotify: {
                XConfigureEvent xce = e.xconfigure;

                // Checking if window has been resized
                if ((u32)xce.width != x11_state->window_attributes.width ||
                    (u32)xce.height != x11_state->window_attributes.height) {
                    x11_resize_window(x11_state, (u32)xce.width, (u32)xce.height);
                    window_resize(application_state,
                                x11_state->window_attributes.width,
                                x11_state->window_attributes.height);
                }
            } break;
            default: {}
        }

        // Add event to game's event buffer if it is a finch event
        if (finch_event.type != FC_EVENT_TYPE_NONE &&
            application_state->unhandled_events < MAX_EVENTS) {
            application_state->events[application_state->unhandled_events++] = finch_event;
        }
    }

    if (mouse_moved && (application_state->unhandled_events < MAX_EVENTS)) {
        FcEvent mouse_moved_event = {0};
        mouse_moved_event.type = FC_EVENT_TYPE_MOUSE_MOVED;
        mouse_moved_event.mouse_x = mouse_x;
        mouse_moved_event.mouse_y = mouse_y;
        mouse_moved_event.mouse_dx = mouse_x - prev_x;
        mouse_moved_event.mouse_dy = mouse_y - prev_y;
        application_state->events[application_state->unhandled_events++] = mouse_moved_event;

        prev_x = mouse_x;
        prev_y = mouse_y;
    }
}

static f64 clock_now(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        FC_ENGINE_ERROR("Could not get current monotonic time: %s",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    return (f64)(now.tv_sec + (now.tv_nsec / 1000) * 0.000001);
}

void platform_init(ApplicationState* application_state)
{
    char* game_name = application_state->name != NULL
        ? application_state->name
        : "Finch Application";
    u32 game_width = application_state->width_px > 0
        ? application_state->width_px
        : 1280;
    u32 game_height = application_state->height_px > 0
        ? application_state->height_px
        : 720;
    WindowAttributes window_attributes = {
        .title  = game_name,
        .width  = game_width,
        .height = game_height
    };

    x11_state.window_attributes = window_attributes;
    x11_init(&x11_state);

    window_resize(application_state,
                x11_state.window_attributes.width,
                x11_state.window_attributes.height);
}

void platform_deinit(ApplicationState* application_state)
{
    UNUSED(application_state);
    x11_deinit(&x11_state);
}

void platform_poll_events(ApplicationState* application_state)
{
    if (x11_state.cursor_centered) {
        u32 new_x = x11_state.window_attributes.width / 2;
        u32 new_y = x11_state.window_attributes.height / 2;
        prev_x = new_x;
        prev_y = new_y;
        XGrabPointer(x11_state.display, XDefaultRootWindow(x11_state.display), 
                     False, 0, GrabModeAsync, GrabModeAsync, 
                     None, None, CurrentTime);
        x11_move_cursor(x11_state.display, x11_state.window,
                        new_x, new_y);
        XUngrabPointer(x11_state.display, CurrentTime);
    }

    b32 should_grab_pointer = x11_state.cursor_confined;
    if (should_grab_pointer) {
        XGrabPointer(x11_state.display, XDefaultRootWindow(x11_state.display),
                     True, PointerMotionMask, GrabModeAsync, GrabModeAsync,
                     x11_state.window, None, CurrentTime);
    }
    
    x11_handle_events(&x11_state, application_state);
    
    if (should_grab_pointer) {
        XUngrabPointer(x11_state.display, CurrentTime);
    }
    
}

WindowAttributes* platform_get_window_attributes(void)
{
    return &x11_state.window_attributes;
}

f64 platform_get_epoch_time(void)
{
    return clock_now();
}

void platform_set_window_title(const char* title)
{
    XStoreName(x11_state.display, x11_state.window, title);
}

b32 platform_terminal_supports_colors()
{
    int fd[2];
    if (pipe(fd) < 0) {
        return false;
    };

    pid_t pid = fork();
    if (pid < 0) {
        return false;
    }

    if (pid == 0) {
        // Child
        close(fd[0]);
        if (dup2(fd[1], STDOUT_FILENO) < 0) {
            return false;
        }
        close(fd[1]);
        if (execl("/bin/tput", "/bin/tput", "colors", NULL) < 0) {
            return false;
        }
    }

    close(fd[1]);
    char buf[100];
    while (read(fd[0], buf, sizeof(buf)) != 0) {};
    return (atoi(buf) == 256);
}

b32 platform_stdout_is_terminal() { return isatty(STDOUT_FILENO) == 1; }

b32 platform_stderr_is_terminal() { return isatty(STDERR_FILENO) == 1; }

void platform_set_terminal_color(FcTerminalColor color) {
    if (terminal_supports_colors == -1) {
        terminal_supports_colors =
            platform_stdout_is_terminal() && platform_terminal_supports_colors();
    }

    if (terminal_supports_colors) {
        switch (color) {
            case FC_TERM_COLOR_WHITE: {
                printf("\033[0m");
            } break;
            case FC_TERM_COLOR_GREEN: {
                printf("\033[32m");
            } break;
            case FC_TERM_COLOR_ORANGE: {
                printf("\033[33m");
            } break;
            case FC_TERM_COLOR_RED: {
                printf("\033[31m");
            } break;
        }
    }
}

void platform_get_framebuffer_size(u32* width, u32* height) { x11_get_framebuffer_size(&x11_state, width, height); }

void platform_set_cursor_style(FcCursorStyle cursor_style)
{
    x11_set_cursor_style(x11_state.display, x11_state.window, cursor_style);
}

void platform_move_cursor(u32 x, u32 y) { x11_move_cursor(x11_state.display, x11_state.window, x, y); }

void platform_toggle_fullscreen(void) { x11_toggle_fullscreen(x11_state.display, x11_state.window); }

void platform_toggle_cursor_confinement(void) { x11_toggle_cursor_confinement(); }

void platform_toggle_cursor_centered(void) { x11_toggle_cursor_centered(); }
