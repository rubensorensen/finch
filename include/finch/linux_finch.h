#ifndef _FINCH_LINUX_FINCH_H
#define _FINCH_LINUX_FINCH_H

#include "finch/core.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

typedef struct _X11State {
    Display *display;
    int      screen;
    Window   window;
    GC       gc;
    Atom     wm_delete_window;

    WindowAttributes window_attributes;
} X11State;

void x11_get_framebuffer_size(X11State* x11_state, u32* width, u32* height);

#endif // _FINCH_LINUX_FINCH_H
