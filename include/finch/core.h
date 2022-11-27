#ifndef _FINCH_CORE_H
#define _FINCH_CORE_H

#include <stdint.h>
#include <stdbool.h>

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

typedef bool b32;

typedef struct _WindowAttributes {
    char* title;
    u32 width, height;
} WindowAttributes;

typedef enum _FcCursorStyle {
    FC_CURSOR_STYLE_DEFAULT = 0,
    FC_CURSOR_STYLE_HIDDEN,
    FC_CURSOR_STYLE_POINTER,
    FC_CURSOR_STYLE_WAIT,
    FC_CURSOR_STYLE_TEXT,
    FC_CURSOR_STYLE_CROSSHAIR,
    FC_CURSOR_STYLE_MOVE,
    FC_CURSOR_STYLE_NOT_ALLOWED,
    FC_CURSOR_STYLE_UP_ARROW,
    FC_CURSOR_STYLE_DOWN_ARROW,
    FC_CURSOR_STYLE_LEFT_ARROW,
    FC_CURSOR_STYLE_RIGHT_ARROW,
    FC_CURSOR_STYLE_UP_DOWN_ARROW,
    FC_CURSOR_STYLE_LEFT_RIGHT_ARROW,

    FC_CURSOR_STYLE_COUNT
} FcCursorStyle;

#endif // _FINCH_CORE_H
