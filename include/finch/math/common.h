#ifndef _FINCH_MATH_COMMON_H
#define _FINCH_MATH_COMMON_H

#include "finch/core/core.h"

#include <math.h>

#define PI  3.14159265358979323846
#define TAU (2*PI)

#define sqrt(x) (f32)sqrtf((float)(x))
#define sin(x)  (f32)sinf((float)(x))
#define cos(x)  (f32)cosf((float)(x))
#define tan(x)  (f32)tanf((float)(x))

f32 radians(f32 deg);

#endif // _FINCH_MATH_COMMON_H
