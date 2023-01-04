#ifndef _FINCH_APPLICATION_H
#define _FINCH_APPLICATION_H

#include "finch/events.h"

#define MAX_EVENTS 1024

typedef union _Color {
    u32 packed;
    struct {
        u8 r, g, b, a;
    };
} Color;

typedef struct _ApplicationState {
    char* name;
    u32  width_px;
    u32  height_px;
    int  running;

    FcEvent events[MAX_EVENTS];
    u32 unhandled_events;
} ApplicationState;

// Implemented by application
void fc_application_init(ApplicationState*);
void fc_application_update(ApplicationState*, f64);
void fc_application_deinit(ApplicationState*);

#endif // _FINCH_APPLICATION_H
