#ifndef FINCH_GAME_H
#define FINCH_GAME_H

#include "events.h"

#define MAX_EVENTS 1000

typedef union _Color {
    u32 packed;
    struct {
        u8 r, g, b, a;
    };
} Color;

typedef struct _InputState {
    b32 button_is_down[FC_BUTTON_COUNT];
    b32 key_is_down[FC_KEY_COUNT];
    u32 mouse_x, mouse_y;
    s32 mouse_dx, mouse_dy;
} InputState;

typedef struct _GameState {
    u32  width_px;
    u32  height_px;
    int  running; 
    u32* pixelbuffer;

    InputState input_state;
    FcEvent events[MAX_EVENTS];
    u32 unhandled_events;
} GameState;

#endif // FINCH_GAME_H
