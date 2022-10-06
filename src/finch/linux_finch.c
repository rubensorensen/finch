#define _POSIX_C_SOURCE 199309L

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "core.h"
#include "events.h"
#include "game.h"

#include "loggy/loggy.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>

static LgLogger logger;

typedef struct _X11State {
    Display *display;
    int      screen;
    Window   window;
    GC       gc;
    Atom     wm_delete_window;

    const char* window_title;
    u32 window_width;
    u32 window_height;
    
    f32 time_since_window_title_updated;
} X11State;

static void x11_init(X11State* x11_state)
{
    x11_state->display = XOpenDisplay(NULL);
    if (x11_state->display == NULL) {
        LG_FATAL(&logger, "Could not open default display.");
    }

    x11_state->screen = DefaultScreen(x11_state->display);
    x11_state->window = XCreateSimpleWindow(x11_state->display,
                                            XDefaultRootWindow(x11_state->display),
                                            0, 0,
                                            x11_state->window_width,
                                            x11_state->window_height,
                                            0, 0,
                                            WhitePixel(x11_state->display, x11_state->screen));

    XStoreName(x11_state->display, x11_state->window, x11_state->window_title);
    x11_state->gc = XCreateGC(x11_state->display, x11_state->window, 0, NULL);

    x11_state->wm_delete_window = XInternAtom(x11_state->display,
                                              "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x11_state->display, x11_state->window, &x11_state->wm_delete_window, 1);

    XSelectInput(x11_state->display, x11_state->window,
                 KeyPressMask | KeyReleaseMask |
                 ButtonPressMask | ButtonReleaseMask |
                 PointerMotionMask |
                 StructureNotifyMask | ExposureMask);

    XMapWindow(x11_state->display, x11_state->window);
}

static void x11_deinit(X11State* x11_state)
{
    XFreeGC(x11_state->display, x11_state->gc);
	XDestroyWindow(x11_state->display, x11_state->window);
    XCloseDisplay(x11_state->display);
}

static void x11_put_pixelbuffer_on_screen(X11State* x11_state, GameState* game_state)
{
    XWindowAttributes window_attributes;
    XGetWindowAttributes(x11_state->display, x11_state->window, &window_attributes);
    
    XImage* img = XCreateImage(x11_state->display,
                               window_attributes.visual, window_attributes.depth,
                               ZPixmap, 0, (char*)game_state->pixelbuffer,
                               game_state->width_px, game_state->height_px,
                               32, game_state->width_px * sizeof(game_state->pixelbuffer[0]));

    XPutImage(x11_state->display, x11_state->window,
              x11_state->gc, img,
              0, 0,
              0, 0,
              game_state->width_px,
              game_state->height_px);
}

static void game_initialize_pixelbuffer(GameState* game_state) {
    if (game_state->pixelbuffer != NULL) {
        free(game_state->pixelbuffer);
    }
    game_state->pixelbuffer = (u32*)malloc(game_state->width_px * game_state->height_px * sizeof(u32));
}

static void game_resize(GameState* game_state, u32 new_width, u32 new_height)
{
    game_state->width_px  = new_width;
    game_state->height_px = new_height;
    game_initialize_pixelbuffer(game_state);
}

static void x11_resize_window(X11State* x11_state, u32 new_width, u32 new_height)
{
    x11_state->window_width  = new_width;
    x11_state->window_height = new_height;
}

static void x11_handle_events(X11State* x11_state, GameState* game_state)
{
    game_state->unhandled_events = 0;
    game_state->input_state.mouse_dx = 0;
    game_state->input_state.mouse_dy = 0;
    while (XPending(x11_state->display) > 0) {
        FcEvent finch_event = {0};
        
        XEvent e = {0};
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
                            LG_WARN(&logger, "Unhandled key press event (Key: %Vd)",
                                    key);
                            finch_event.type = FC_EVENT_TYPE_NONE;
                            finch_key = FC_KEY_NONE;
                        }
                    }
                }
                
                finch_event.key = finch_key;
                game_state->input_state.key_is_down[finch_key] = true;
                
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
                game_state->input_state.key_is_down[finch_key] = false;
                
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
                game_state->input_state.button_is_down[finch_button] = true;
                
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
                game_state->input_state.button_is_down[finch_button] = false;
                
            } break;
            case MotionNotify: {
                finch_event.type = FC_EVENT_TYPE_MOUSE_MOVED;
                
                finch_event.mouse_x = e.xbutton.x;
                game_state->input_state.mouse_dx +=
                    (e.xbutton.x - game_state->input_state.mouse_x);
                
                game_state->input_state.mouse_x = e.xmotion.x;
                
                finch_event.mouse_y = e.xbutton.y;
                game_state->input_state.mouse_dy +=
                    (e.xbutton.y - game_state->input_state.mouse_y);
                
                game_state->input_state.mouse_y = e.xmotion.y;
                
            } break;
            case ClientMessage: {
                if ((Atom)e.xclient.data.l[0] == x11_state->wm_delete_window) {
                    game_state->running = false;
                }
            } break;
            case Expose: {
                x11_put_pixelbuffer_on_screen(x11_state, game_state);
            } break;
            case ConfigureNotify: {
                XConfigureEvent xce = e.xconfigure;

                // Checking if window has been resized
                if ((u32)xce.width != x11_state->window_width ||
                    (u32)xce.height != x11_state->window_height) {
                    x11_resize_window(x11_state, (u32)xce.width, (u32)xce.height);
                    game_resize(game_state, x11_state->window_width, x11_state->window_height);
                } 
            } break;
        }

        // Add event to game's event buffer if it is a finch event
        if (finch_event.type != FC_EVENT_TYPE_NONE &&
            game_state->unhandled_events < MAX_EVENTS) {
            game_state->events[game_state->unhandled_events++] = finch_event;
        }
    }
}

static f64 clock_now(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        LG_FATAL(&logger, "Could not get current monotonic time: %s",
                strerror(errno));
    }
    return (f64)(now.tv_sec + (now.tv_nsec / 1000) * 0.000001);
}

static void* sandbox_so;
static void (*game_update) (GameState*, f32);
static time_t sandbox_last_change;

static void load_game_if_changed(void)
{
    struct stat statbuf;
    if (stat("./libsandbox.so", &statbuf) < 0) {
        LG_FATAL(&logger, "Could not read file stats from ./libsandbox.so: %s",
                strerror(errno));
    }
    time_t time = statbuf.st_mtime;

    if (time != sandbox_last_change) {
        sandbox_last_change = time;
        if (sandbox_so != NULL) {
            dlclose(sandbox_so);
            sandbox_so = NULL;
        }

        sandbox_so = dlopen("./libsandbox.so", RTLD_LAZY);
        if (sandbox_so == NULL) {
            LG_FATAL(&logger, "Could not open /tmp/libsandbox.so: %s",
                    dlerror());
        }
    
        game_update = dlsym(sandbox_so, "game_update");
        if (game_update == NULL) {
            LG_FATAL(&logger, "Could not load game_update function from /tmp/libsandbox.so: %s",
                    dlerror());
            exit(EXIT_FAILURE);
        }
    }
}

int main(void)
{
    loggy_init(&logger, "%TT (%Fn:%Fl) %CcFINCH%Cn [%Gl] ");
    
    load_game_if_changed();
    
    X11State x11_state = {0};
    x11_state.window_title  = "Finch";
    x11_state.window_width  = 1280;
    x11_state.window_height = 720;
    x11_init(&x11_state);
    
    GameState game_state = {};
    loggy_init(&game_state.logger, "%TT (%Fn:%Fl) %CcSANDBOX%Cn [%Gl] ");
    game_resize(&game_state, x11_state.window_width, x11_state.window_height);

    f64 prev_time = clock_now();
    
    game_state.running = true;
    while (game_state.running) {
        load_game_if_changed();
        f64 curr_time = clock_now();
        f64 delta_time = curr_time - prev_time;
        
        x11_handle_events(&x11_state, &game_state);
        (*game_update)(&game_state, delta_time);
        x11_put_pixelbuffer_on_screen(&x11_state, &game_state);

        // Update fps in window title approx. every second
        x11_state.time_since_window_title_updated += delta_time;
        if (x11_state.time_since_window_title_updated > 1.0f) {
            char buf[1000];
            sprintf(buf, "%s - %dfps", x11_state.window_title, (u32)(1.0 / delta_time));
            XStoreName(x11_state.display, x11_state.window, buf);
            x11_state.time_since_window_title_updated = 0.0f;
        }

        prev_time = curr_time;
    }

    // Program is finished, free resources and exit
    
    x11_deinit(&x11_state);
    if (game_state.pixelbuffer) {
        free(game_state.pixelbuffer);
    }
    
    dlclose(sandbox_so);
    sandbox_so = NULL;
    
    return EXIT_SUCCESS;
}
