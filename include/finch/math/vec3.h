#ifndef _FINCH_MATH_VEC3_H
#define _FINCH_MATH_VEC3_H

#include "finch/core/core.h"

typedef struct _Vec3F32 {
    union {
        struct {
            f32 x, y, z;
        };
        struct {
            f32 r, g, b;
        };
    };
} Vec3F32;

#endif // _FINCH_MATH_VEC3_H
