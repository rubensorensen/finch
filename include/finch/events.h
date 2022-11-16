#ifndef _FINCH_EVENTS_H
#define _FINCH_EVENTS_H

#include "finch/core.h"

typedef enum _FcEventType {
    FC_EVENT_TYPE_NONE = 0,
    FC_EVENT_TYPE_BUTTON_PRESSED, FC_EVENT_TYPE_BUTTON_RELEASED,
    FC_EVENT_TYPE_KEY_PRESSED, FC_EVENT_TYPE_KEY_RELEASED,
    FC_EVENT_TYPE_WHEEL_SCROLLED, FC_EVENT_TYPE_MOUSE_MOVED,

    FC_EVENT_TYPE_COUNT
} FcEventType;

typedef enum _FcButton {
    FC_BUTTON_NONE = 0,
    FC_BUTTON_LEFT,
    FC_BUTTON_RIGHT,
    FC_BUTTON_MIDDLE,

    FC_BUTTON_COUNT
} FcButton;

typedef enum _FcKey {
    FC_KEY_NONE = 0,

    FC_KEY_A,
    FC_KEY_B,
    FC_KEY_C,
    FC_KEY_D,
    FC_KEY_E,
    FC_KEY_F,
    FC_KEY_G,
    FC_KEY_H,
    FC_KEY_I,
    FC_KEY_J,
    FC_KEY_K,
    FC_KEY_L,
    FC_KEY_M,
    FC_KEY_N,
    FC_KEY_O,
    FC_KEY_P,
    FC_KEY_Q,
    FC_KEY_R,
    FC_KEY_S,
    FC_KEY_T,
    FC_KEY_U,
    FC_KEY_V,
    FC_KEY_W,
    FC_KEY_X,
    FC_KEY_Y,
    FC_KEY_Z,

    FC_KEY_0,
    FC_KEY_1,
    FC_KEY_3,
    FC_KEY_4,
    FC_KEY_5,
    FC_KEY_6,
    FC_KEY_7,
    FC_KEY_8,
    FC_KEY_9,

    FC_KEY_LEFT_CTRL,
    FC_KEY_RIGHT_CTRL,

    FC_KEY_LEFT_SHIFT,
    FC_KEY_RIGHT_SHIFT,

    FC_KEY_LEFT_ALT,
    FC_KEY_RIGHT_ALT,

    FC_KEY_SPACE,
    FC_KEY_TAB,
    FC_KEY_ESC,
    FC_KEY_ENTER,
    FC_KEY_BACKSPACE,

    FC_KEY_CAPS_LOCK,

    FC_KEY_UP,
    FC_KEY_DOWN,
    FC_KEY_LEFT,
    FC_KEY_RIGHT,

    FC_KEY_F1,
    FC_KEY_F2,
    FC_KEY_F3,
    FC_KEY_F4,
    FC_KEY_F5,
    FC_KEY_F6,
    FC_KEY_F7,
    FC_KEY_F8,
    FC_KEY_F9,
    FC_KEY_F10,
    FC_KEY_F11,
    FC_KEY_F12,

    FC_KEY_COUNT
} FcKey;

typedef struct _FcEvent {
    FcEventType type;
    FcButton button;
    FcKey key;
    u32 mouse_x, mouse_y;
    s8 scroll_wheel_vertical_direction;
    s8 scroll_wheel_horizontal_direction;
} FcEvent;

#endif // _FINCH_EVENTS_H
