#define _POSIX_C_SOURCE 199309L

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

typedef struct X11State {
    Display *display;
    int      screen;
    Window   window;
    GC       gc;
    Atom     wm_delete_window;

    u32 window_width;
    u32 window_height;
} X11State;


static void x11_init(X11State* x11_state)
{
    x11_state->window_width  = 1280;
    x11_state->window_height = 720;
    x11_state->display = XOpenDisplay(NULL);
    if (x11_state->display == NULL) {
        fprintf(stderr, "[ERROR] Could not open default display.\n");
        exit(1);
    }

    x11_state->screen = DefaultScreen(x11_state->display);
    x11_state->window = XCreateSimpleWindow(x11_state->display,
                                            XDefaultRootWindow(x11_state->display),
                                            0, 0,
                                            x11_state->window_width,
                                            x11_state->window_height,
                                            0, 0,
                                            WhitePixel(x11_state->display, x11_state->screen));

    XStoreName(x11_state->display, x11_state->window, "Finch");
    x11_state->gc = XCreateGC(x11_state->display, x11_state->window, 0, NULL);

    x11_state->wm_delete_window = XInternAtom(x11_state->display,
                                              "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x11_state->display, x11_state->window, &x11_state->wm_delete_window, 1);

    XSelectInput(x11_state->display, x11_state->window, KeyPressMask | StructureNotifyMask | ExposureMask);

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
    // Rendering
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
    while (XPending(x11_state->display) > 0) {
        XEvent e = {0};
        XNextEvent(x11_state->display, &e);
        
        switch (e.type) {
            case KeyPress: {
                KeySym key = XLookupKeysym(&e.xkey, 0);
                switch (key) {
                    case 'q': {
                        game_state->running = 0;
                    } break;
                    default: {
                        printf("You pressed key '%c'\n", (char)key);
                    }
                }
            } break;
            case ClientMessage: {
                if ((Atom)e.xclient.data.l[0] == x11_state->wm_delete_window) {
                    game_state->running = 0;
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
    }
}

static f32 clock_now()
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) < 0) {
        fprintf(stderr, "[ERROR] could not get current monotonic time: %s\n",
                strerror(errno));
        exit(1);
    }
    return (f32)(now.tv_sec + (now.tv_nsec / 1000) * 0.000001);
}

extern void game_update(GameState*, f32);

int main(void)
{
    X11State x11_state = {0};
    x11_init(&x11_state);
    
    GameState game_state = {};
    game_resize(&game_state, x11_state.window_width, x11_state.window_height);

    f32 prev_time = clock_now();
    
    game_state.running = 1;
    while (game_state.running) {
        
        f32 curr_time = clock_now();
        x11_handle_events(&x11_state, &game_state);
        game_update(&game_state, curr_time - prev_time);
        x11_render(&x11_state, &game_state);

        prev_time = curr_time;
    }

    x11_deinit(&x11_state);
    return 0;
}
