#ifndef FINCH_APPLICATION_APPLICATION_H
#define FINCH_APPLICATION_APPLICATION_H

#include "finch/core/events.h"

#define MAX_EVENTS 1024

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

typedef struct _ApplicationState {
    char* name;
    u32  width_px;
    u32  height_px;
    int  running;

    InputState input_state;
    FcEvent events[MAX_EVENTS];
    u32 unhandled_events;
} ApplicationState;

// Implemented by application
void fc_application_init(ApplicationState*);
void fc_application_update(ApplicationState*, f64);
void fc_application_deinit(ApplicationState*);

#endif // FINCH_APPLICATION_APPLICATION_H
