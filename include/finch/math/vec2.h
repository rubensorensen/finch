#ifndef _FINCH_MATH_VEC2_H
#define _FINCH_MATH_VEC2_H

#include "finch/core/core.h"

typedef struct _Vec2F32 {
    union {
        struct {
            f32 x, y;
        };
        struct {
            f32 r, g;
        };
    };
} Vec2F32;

#endif // _FINCH_MATH_VEC2_H
