#ifndef FINCH_GAME_H
#define FINCH_GAME_H

#include "core.h"
#include "events.h"

#define MAX_EVENTS 1000

typedef struct _GameState {
    u32  width_px;
    u32  height_px;
    int  running; 
    u32* pixelbuffer;
    
    FcEvent events[MAX_EVENTS];
    u32 unhandled_events;
} GameState;

#endif // FINCH_GAME_H
