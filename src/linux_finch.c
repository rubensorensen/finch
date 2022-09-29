#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef float  f32;
typedef double f64;

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

typedef struct X11State {
    Display *display;
    int      screen;
    Window   window;
    GC       gc;
    Atom     wm_delete_window;    
} X11State;

typedef struct GameState {
    int  running; 
    u32* pixelbuffer;
} GameState;

typedef union Color {
    u32 packed;
    struct {
        u8 r, g, b, a;
    };
} Color;

static u32 format_color(Color col)
{
    return (u32)(col.a << 24 |
                 col.r << 16 |
                 col.g << 8  |
                 col.b << 0);
}

static void init_x11(X11State* x11_state)
{
    x11_state->display = XOpenDisplay(NULL);
    if (x11_state->display == NULL) {
        fprintf(stderr, "[ERROR] Could not open default display.\n");
        exit(1);
    }

    x11_state->screen = DefaultScreen(x11_state->display);
    x11_state->window = XCreateSimpleWindow(x11_state->display,
                                            XDefaultRootWindow(x11_state->display),
                                            0, 0,
                                            WINDOW_WIDTH, WINDOW_HEIGHT,
                                            0, 0,
                                            WhitePixel(x11_state->display, x11_state->screen));

    x11_state->gc = XCreateGC(x11_state->display, x11_state->window, 0, NULL);

    x11_state->wm_delete_window = XInternAtom(x11_state->display,
                                              "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x11_state->display, x11_state->window, &x11_state->wm_delete_window, 1);

    XSelectInput(x11_state->display, x11_state->window, KeyPressMask);

    XMapWindow(x11_state->display, x11_state->window);
}

static void deinit_x11(X11State* x11_state)
{
    XFreeGC(x11_state->display, x11_state->gc);
	XDestroyWindow(x11_state->display, x11_state->window);
    XCloseDisplay(x11_state->display);    
}

static void handle_events(X11State* x11_state, GameState* game_state)
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
        }
    }
}

static void render(X11State* x11_state, GameState* game_state)
{
    // Rendering
    XWindowAttributes window_attributes;
    XGetWindowAttributes(x11_state->display, x11_state->window, &window_attributes);
    XImage* img = XCreateImage(x11_state->display,
                               window_attributes.visual, window_attributes.depth,
                               ZPixmap, 0, (char*)game_state->pixelbuffer,
                               WINDOW_WIDTH, WINDOW_HEIGHT,
                               32, WINDOW_WIDTH * sizeof(game_state->pixelbuffer[0]));

    XPutImage(x11_state->display, x11_state->window,
              x11_state->gc, img,
              0, 0,
              0, 0,
              WINDOW_WIDTH,
              WINDOW_HEIGHT);
}

int main(void)
{
    GameState game_state = {};
    game_state.pixelbuffer = (u32*)malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(u32));

    for (int j = 0; j < WINDOW_HEIGHT; ++j) {
        for (int i = 0; i < WINDOW_WIDTH; ++i) {
            f32 u = i / (f32)WINDOW_WIDTH;
            f32 v = j / (f32)WINDOW_HEIGHT;

            Color col = {0};
            col.r = u * 255;
            col.g = v * 255;
            col.b = 0xFFu;
            col.a = 0xFFu;            

            u32 color = format_color(col);
            game_state.pixelbuffer[i + j * WINDOW_WIDTH] = color;
        }
    }

    X11State x11_state = {0};
    init_x11(&x11_state);
    
    game_state.running = 1;
    while (game_state.running) {
        handle_events(&x11_state, &game_state);
        render(&x11_state, &game_state);
    }

    deinit_x11(&x11_state);
    return 0;
}
