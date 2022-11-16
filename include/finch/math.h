#ifndef _FINCH_MATH_H
#define _FINCH_MATH_H

#include "finch/core.h"

#include <math.h>

#define PI  3.14159265358979323846
#define TAU (2*PI)

#define sqrt(x) (f32)sqrtf((float)(x))
#define sin(x)  (f32)sinf((float)(x))
#define cos(x)  (f32)cosf((float)(x))
#define tan(x)  (f32)tanf((float)(x))

f32 radians(f32 deg);

typedef union _v2f32 {
    f32 c[2];
    struct {
        union { f32 x, u; };
        union { f32 y, v; };
    };
} v2f32;

typedef union _v3f32 {
    f32 c[3];
    struct {
        union { f32 x, r; };
        union { f32 y, g; };
        union { f32 z, b; };
    };
} v3f32;

typedef union _v4f32 {
    f32 c[4];
    struct {
        union { f32 x, r; };
        union { f32 y, g; };
        union { f32 z, b; };
        union { f32 w, a; };
    };
} v4f32;

typedef union _m4x4f32 {
    f32 c[16];
    v4f32 cols[4];
} m4f32;

#define X_AXIS (v3f32){{1.0f, 0.0f, 0.0f}}
#define Y_AXIS (v3f32){{0.0f, 1.0f, 0.0f}}
#define Z_AXIS (v3f32){{0.0f, 0.0f, 1.0f}}

#define X_AXIS_INV (v3f32){{-1.0f, 0.0f, 0.0f}}
#define Y_AXIS_INV (v3f32){{0.0f, -1.0f, 0.0f}}
#define Z_AXIS_INV (v3f32){{0.0f, 0.0f, -1.0f}}

// Debug
void v2f32_print(v2f32 v);
void v3f32_print(v3f32 v);
void v4f32_print(v4f32 v);
void m4f32_print(m4f32 m);

//
// 2D Vector functions
//

// Scalar arithmetic
v2f32 v2f32_add_f32(v2f32 v, f32 s);
v2f32 v2f32_sub_f32(v2f32 v, f32 s);
v2f32 v2f32_mul_f32(v2f32 v, f32 s);
v2f32 v2f32_div_f32(v2f32 v, f32 s);

// Vector arithmetic
v2f32 v2f32_add_v2f32(v2f32 u, v2f32 v);
v2f32 v2f32_sub_v2f32(v2f32 u, v2f32 v);
v2f32 v2f32_mul_v2f32(v2f32 u, v2f32 v);
v2f32 v2f32_div_v2f32(v2f32 u, v2f32 v);

// Etc
f32 v2f32_length(v2f32 v);
f32 v2f32_length_sq(v2f32 v);
v2f32 v2f32_normalize(v2f32 v);
f32 v2f32_dot(v2f32 u, v2f32 v);

//
// 3D Vector functions
//

// Scalar arithmetic
v3f32 v3f32_add_f32(v3f32 v, f32 s);
v3f32 v3f32_sub_f32(v3f32 v, f32 s);
v3f32 v3f32_mul_f32(v3f32 v, f32 s);
v3f32 v3f32_div_f32(v3f32 v, f32 s);

// Vector arithmetic
v3f32 v3f32_add_v3f32(v3f32 u, v3f32 v);
v3f32 v3f32_sub_v3f32(v3f32 u, v3f32 v);
v3f32 v3f32_mul_v3f32(v3f32 u, v3f32 v);
v3f32 v3f32_div_v3f32(v3f32 u, v3f32 v);

// Etc
f32 v3f32_length(v3f32 v);
f32 v3f32_length_sq(v3f32 v);
v3f32 v3f32_normalize(v3f32 v);
f32 v3f32_dot(v3f32 u, v3f32 v);
v3f32 v3f32_cross(v3f32 u, v3f32 v);

//
// 4D Vector functions
//

// Scalar arithmetic
v4f32 v4f32_add_f32(v4f32 v, f32 s);
v4f32 v4f32_sub_f32(v4f32 v, f32 s);
v4f32 v4f32_mul_f32(v4f32 v, f32 s);
v4f32 v4f32_div_f32(v4f32 v, f32 s);

// Vector arithmetic
v4f32 v4f32_add_v4f32(v4f32 u, v4f32 v);
v4f32 v4f32_sub_v4f32(v4f32 u, v4f32 v);
v4f32 v4f32_mul_v4f32(v4f32 u, v4f32 v);
v4f32 v4f32_div_v4f32(v4f32 u, v4f32 v);

// Etc
f32 v4f32_length(v4f32 v);
f32 v4f32_length_sq(v4f32 v);
v4f32 v4f32_normalize(v4f32 v);
f32 v4f32_dot(v4f32 u, v4f32 v);


//
// Matrix 4x4 functions
//

m4f32 m4f32_identity();

// Transform
m4f32 m4f32_translate(m4f32 m, v3f32 v);
m4f32 m4f32_rotate(m4f32 matrix, f32 angle, v3f32 axis);
m4f32 m4f32_scale(m4f32 m, v3f32 v);
m4f32 m4f32_look_at(v3f32 eye, v3f32 center, v3f32 up);

// Projection
m4f32 m4f32_perspective(f32 fovy, f32 aspect_ratio, f32 z_near, f32 z_far);

#endif // _FINCH_MATH_H
