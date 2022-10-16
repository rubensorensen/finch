#ifndef LINUX_FINCH_H
#define LINUX_FINCH_H

#include "finch/core/core.h"

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

#endif // LINUX_FINCH_H
