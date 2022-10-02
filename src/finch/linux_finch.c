#define _POSIX_C_SOURCE 199309L

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "core.h"
#include "events.h"
#include "game.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/stat.h>

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
        fprintf(stderr, "[ERROR] Could not open default display.\n");
        exit(EXIT_FAILURE);
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

static void x11_render(X11State* x11_state, GameState* game_state)
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
    while (XPending(x11_state->display) > 0) {
        FcEvent finch_event = {0};
        
        XEvent e = {0};
        XNextEvent(x11_state->display, &e);
        
        switch (e.type) {
            case KeyPress: {
                finch_event.type = FC_EVENT_TYPE_KEY_PRESSED;
                KeySym key = XLookupKeysym(&e.xkey, 0);
                
                // Character key
                if (key >= 'a' && key <= 'z') {
                    finch_event.key = (FcKey)(key - 'a' + FC_KEY_A);
                    break;
                }

                // Numeric key
                if (key >= '0' && key <= '9') {
                    finch_event.key = (FcKey)(key - '0' + FC_KEY_0);
                    break;
                }

                // Special keys
                switch (key) {
                    case 32: {
                        finch_event.key = FC_KEY_SPACE;
                    } break;
                    case 65507: {
                        finch_event.key = FC_KEY_LEFT_CTRL;
                    } break;
                    case 65508: {
                        finch_event.key = FC_KEY_RIGHT_CTRL;
                    } break;
                    case 65505: {
                        finch_event.key = FC_KEY_LEFT_SHIFT;
                    } break;
                    case 65506: {
                        finch_event.key = FC_KEY_RIGHT_SHIFT;
                    } break;
                    case 65513: {
                        finch_event.key = FC_KEY_LEFT_ALT;
                    } break;
                    case 65027: {
                        finch_event.key = FC_KEY_RIGHT_ALT;
                    } break;
                    case 65289: {
                        finch_event.key = FC_KEY_TAB;
                    } break;
                    case 65307: {
                        finch_event.key = FC_KEY_ESC;
                    } break;
                    case 65293: {
                        finch_event.key = FC_KEY_ENTER;
                    } break;
                    case 65288: {
                        finch_event.key = FC_KEY_BACKSPACE;
                    } break;
                    default: {
                        // Unhandled key
                        finch_event.type = FC_EVENT_TYPE_NONE;
                    }
                }
                
            } break;
            case KeyRelease: {

                // Remove auto-generated key-releases and key-presses
                // generated from repeats.
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

                // Character key
                if (key >= 'a' && key <= 'z') {
                    finch_event.key = (FcKey)(key - 'a' + FC_KEY_A);
                    break;
                }

                // Numeric key
                if (key >= '0' && key <= '9') {
                    finch_event.key = (FcKey)(key - '0' + FC_KEY_0);
                    break;
                }

                // Special keys
                switch (key) {
                    case 32: {
                        finch_event.key = FC_KEY_SPACE;
                    } break;
                    case 65507: {
                        finch_event.key = FC_KEY_LEFT_CTRL;
                    } break;
                    case 65508: {
                        finch_event.key = FC_KEY_RIGHT_CTRL;
                    } break;
                    case 65505: {
                        finch_event.key = FC_KEY_LEFT_SHIFT;
                    } break;
                    case 65506: {
                        finch_event.key = FC_KEY_RIGHT_SHIFT;
                    } break;
                    case 65513: {
                        finch_event.key = FC_KEY_LEFT_ALT;
                    } break;
                    case 65027: {
                        finch_event.key = FC_KEY_RIGHT_ALT;
                    } break;
                    case 65289: {
                        finch_event.key = FC_KEY_TAB;
                    } break;
                    case 65307: {
                        finch_event.key = FC_KEY_ESC;
                    } break;
                    case 65293: {
                        finch_event.key = FC_KEY_ENTER;
                    } break;
                }
            } break;
            case ButtonPress: {
                u32 button = e.xbutton.button;
                
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
                        finch_event.button = FC_BUTTON_LEFT;
                    } break;
                    case 2: {
                        // Middle mouse button
                        finch_event.button = FC_BUTTON_MIDDLE;
                    } break;
                    case 3: {
                        // Right mouse button
                        finch_event.button = FC_BUTTON_RIGHT;
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
                        printf("Unhandled button: %u\n", button);
                    }
                }
            } break;
            case ButtonRelease: {
                u32 button = e.xbutton.button;
                if (button < 1 || button > 3) {
                    break;
                }
                
                finch_event.type = FC_EVENT_TYPE_BUTTON_RELEASED;
                finch_event.mouse_x = e.xbutton.x;
                finch_event.mouse_y = e.xbutton.y;
                
                switch (button) {
                    case 1: {
                        // Left mouse button
                        finch_event.button = FC_BUTTON_LEFT;
                    } break;
                    case 2: {
                        // Middle mouse button
                        finch_event.button = FC_BUTTON_MIDDLE;
                    } break;
                    case 3: {
                        // Right mouse button
                        finch_event.button = FC_BUTTON_RIGHT;
                    } break;
                }
            } break;
            case MotionNotify: {
                // TODO: MotionNotify seems to be inacurrate. Look into this!
                finch_event.type = FC_EVENT_TYPE_MOUSE_MOVED;
                finch_event.mouse_x = e.xmotion.x;
                finch_event.mouse_y = e.xmotion.y;
            } break;
            case ClientMessage: {
                if ((Atom)e.xclient.data.l[0] == x11_state->wm_delete_window) {
                    game_state->running = false;
                }
            } break;
            case Expose: {
                x11_render(x11_state, game_state);
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

static f32 clock_now(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        fprintf(stderr, "[ERROR] could not get current monotonic time: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    return (f32)(now.tv_sec + (now.tv_nsec / 1000) * 0.000001);
}

static void* sandbox_so;
static void (*game_update) (GameState*, f32);
static time_t sandbox_last_change;

static void load_game_if_changed(void)
{
    struct stat statbuf;
    if (stat("./libsandbox.so", &statbuf) < 0) {
        fprintf(stderr, "[ERROR] Could not read file stats from ./libsandbox.so: %s\n",
                strerror(errno));
        exit(EXIT_FAILURE);
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
            fprintf(stderr, "[ERROR] Could not open /tmp/libsandbox.so: %s\n",
                    dlerror());
        }
    
        game_update = dlsym(sandbox_so, "game_update");
        if (game_update == NULL) {
            fprintf(stderr, "[ERROR] Could not load game_update function from /tmp/libsandbox.so: %s\n",
                    dlerror());
            exit(EXIT_FAILURE);
        }
    }
}

int main(void)
{
    load_game_if_changed();
    X11State x11_state = {0};
    x11_state.window_title  = "Finch";
    x11_state.window_width  = 1280;
    x11_state.window_height = 720;
    x11_init(&x11_state);
    
    GameState game_state = {};
    game_resize(&game_state, x11_state.window_width, x11_state.window_height);

    f32 prev_time = clock_now();
    
    game_state.running = true;
    while (game_state.running) {
        load_game_if_changed();
        f32 curr_time = clock_now();
        f32 delta_time = curr_time - prev_time;
        
        x11_handle_events(&x11_state, &game_state);
        (*game_update)(&game_state, delta_time);
        x11_render(&x11_state, &game_state);

        // Update fps in window title approx. every second
        x11_state.time_since_window_title_updated += delta_time;
        if (x11_state.time_since_window_title_updated > 1.0f) {
            char buf[1000];
            sprintf(buf, "%s - %dfps", x11_state.window_title, (u32)(1.0f / delta_time));
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
