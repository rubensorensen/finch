#include "finch/math/lin_alg.h"

#include <stdio.h>

//
// Debug
//

void
v2f32_print(v2f32 v)
{
    printf("(");
    for (u32 i = 0; i < 2; ++i) {
        printf("%f", v.c[i]);
        if (i < 1) {
            printf(", ");
        }
    }
    printf(")\n");
}

void
v3f32_print(v3f32 v)
{
    printf("(");
    for (u32 i = 0; i < 3; ++i) {
        printf("%f", v.c[i]);
        if (i < 2) {
            printf(", ");
        }
    }
    printf(")\n");
}

void
v4f32_print(v4f32 v)
{
    printf("(");
    for (u32 i = 0; i < 4; ++i) {
        printf("%f", v.c[i]);
        if (i < 3) {
            printf(", ");
        }
    }
    printf(")\n");
}

void
m4x4f32_print(m4x4f32 m)
{
    for (u32 j = 0; j < 4; ++j) {
        for (u32 i = 0; i < 4; ++i) {
            printf("%f", m.c[i + j*4]);
            if (i < 3) {
                printf(", ");
            }
        }
        printf("\n");
    }
}

//
// 2D Vector functions
//

v2f32
v2f32_add_f32(v2f32 v, f32 s)
{
    for (u32 i = 0; i < 2; ++i) {
        v.c[i] += s;
    }
    return v;
}

v2f32
v2f32_sub_f32(v2f32 v, f32 s)
{
    for (u32 i = 0; i < 2; ++i) {
        v.c[i] -= s;
    }
    return v;
}

v2f32
v2f32_mul_f32(v2f32 v, f32 s)
{
    for (u32 i = 0; i < 2; ++i) {
        v.c[i] *= s;
    }
    return v;
}

v2f32
v2f32_div_f32(v2f32 v, f32 s)
{
    for (u32 i = 0; i < 2; ++i) {
        v.c[i] /= s;
    }
    return v;
}

v2f32
v2f32_add_v2f32(v2f32 u, v2f32 v)
{
    for (u32 i = 0; i < 2; ++i) {
        u.c[i] += v.c[i];
    }
    return u;
}

v2f32
v2f32_sub_v2f32(v2f32 u, v2f32 v)
{
    for (u32 i = 0; i < 2; ++i) {
        u.c[i] -= v.c[i];
    }
    return u;
}

v2f32
v2f32_mul_v2f32(v2f32 u, v2f32 v)
{
    for (u32 i = 0; i < 2; ++i) {
        u.c[i] *= v.c[i];
    }
    return u;
}

v2f32
v2f32_div_v2f32(v2f32 u, v2f32 v)
{
    for (u32 i = 0; i < 2; ++i) {
        u.c[i] /= v.c[i];
    }
    return u;
}

f32
v2f32_length(v2f32 v)
{
    f32 result = 0.0f;
    for (u32 i = 0; i < 2; ++i) {
        result += v.c[i] * v.c[i];
    }
    return sqrtf(result);
}

f32
v2f32_length_sq(v2f32 v)
{
    f32 result = 0.0f;
    for (u32 i = 0; i < 2; ++i) {
        result += v.c[i] * v.c[i];
    }
    return result;
}

v2f32
v2f32_normalize(v2f32 v)
{
    f32 l = v2f32_length(v);
    for (u32 i = 0; i < 2; ++i) {
        v.c[i] /= l;
    }
    return v;
}

f32
v2f32_dot(v2f32 u, v2f32 v)
{
    f32 result = 0;
    for (u32 i = 0; i < 2; ++i) {
        result += u.c[i] * v.c[i];
    }
    return result;
}

//
// 3D Vector functions
//

v3f32
v3f32_add_f32(v3f32 v, f32 s)
{
    for (u32 i = 0; i < 3; ++i) {
        v.c[i] += s;
    }
    return v;
}

v3f32
v3f32_sub_f32(v3f32 v, f32 s)
{
    for (u32 i = 0; i < 3; ++i) {
        v.c[i] -= s;
    }
    return v;
}

v3f32
v3f32_mul_f32(v3f32 v, f32 s)
{
    for (u32 i = 0; i < 3; ++i) {
        v.c[i] *= s;
    }
    return v;
}

v3f32
v3f32_div_f32(v3f32 v, f32 s)
{
    for (u32 i = 0; i < 3; ++i) {
        v.c[i] /= s;
    }
    return v;
}

v3f32
v3f32_add_v3f32(v3f32 u, v3f32 v)
{
    for (u32 i = 0; i < 3; ++i) {
        u.c[i] += v.c[i];
    }
    return u;
}

v3f32
v3f32_sub_v3f32(v3f32 u, v3f32 v)
{
    for (u32 i = 0; i < 3; ++i) {
        u.c[i] -= v.c[i];
    }
    return u;
}

v3f32
v3f32_mul_v3f32(v3f32 u, v3f32 v)
{
    for (u32 i = 0; i < 3; ++i) {
        u.c[i] *= v.c[i];
    }
    return u;
}

v3f32
v3f32_div_v3f32(v3f32 u, v3f32 v)
{
    for (u32 i = 0; i < 3; ++i) {
        u.c[i] /= v.c[i];
    }
    return u;
}

f32
v3f32_length(v3f32 v)
{
    f32 result = 0.0f;
    for (u32 i = 0; i < 3; ++i) {
        result += v.c[i] * v.c[i];
    }
    return sqrtf(result);
}

f32
v3f32_length_sq(v3f32 v)
{
    f32 result = 0.0f;
    for (u32 i = 0; i < 3; ++i) {
        result += v.c[i] * v.c[i];
    }
    return result;
}

v3f32
v3f32_normalize(v3f32 v)
{
    f32 l = v3f32_length(v);
    for (u32 i = 0; i < 3; ++i) {
        v.c[i] /= l;
    }
    return v;
}

f32
v3f32_dot(v3f32 u, v3f32 v)
{
    f32 result = 0;
    for (u32 i = 0; i < 3; ++i) {
        result += u.c[i] * v.c[i];
    }
    return result;
}

v3f32
v3f32_cross(v3f32 u, v3f32 v)
{
    v3f32 result = {0};
    result.x = u.y * v.z - u.z * v.y;
    result.y = u.z * v.x - u.x * v.z;
    result.z = u.x * v.y - u.y * v.x;
    return result;
}

//
// 4D Vector functions
//

v4f32
v4f32_add_f32(v4f32 v, f32 s)
{
    for (u32 i = 0; i < 4; ++i) {
        v.c[i] += s;
    }
    return v;
}

v4f32
v4f32_sub_f32(v4f32 v, f32 s)
{
    for (u32 i = 0; i < 4; ++i) {
        v.c[i] -= s;
    }
    return v;
}

v4f32
v4f32_mul_f32(v4f32 v, f32 s)
{
    for (u32 i = 0; i < 4; ++i) {
        v.c[i] *= s;
    }
    return v;
}

v4f32
v4f32_div_f32(v4f32 v, f32 s)
{
    for (u32 i = 0; i < 4; ++i) {
        v.c[i] /= s;
    }
    return v;
}

v4f32
v4f32_add_v4f32(v4f32 u, v4f32 v)
{
    for (u32 i = 0; i < 4; ++i) {
        u.c[i] += v.c[i];
    }
    return u;
}

v4f32
v4f32_sub_v4f32(v4f32 u, v4f32 v)
{
    for (u32 i = 0; i < 4; ++i) {
        u.c[i] -= v.c[i];
    }
    return u;
}

v4f32
v4f32_mul_v4f32(v4f32 u, v4f32 v)
{
    for (u32 i = 0; i < 4; ++i) {
        u.c[i] *= v.c[i];
    }
    return u;
}

v4f32
v4f32_div_v4f32(v4f32 u, v4f32 v)
{
    for (u32 i = 0; i < 4; ++i) {
        u.c[i] /= v.c[i];
    }
    return u;
}

f32
v4f32_length(v4f32 v)
{
    f32 result = 0.0f;
    for (u32 i = 0; i < 4; ++i) {
        result += v.c[i] * v.c[i];
    }
    return sqrtf(result);
}

f32
v4f32_length_sq(v4f32 v)
{
    f32 result = 0.0f;
    for (u32 i = 0; i < 4; ++i) {
        result += v.c[i] * v.c[i];
    }
    return result;
}

v4f32
v4f32_normalize(v4f32 v)
{
    f32 l = v4f32_length(v);
    for (u32 i = 0; i < 4; ++i) {
        v.c[i] /= l;
    }
    return v;
}

f32
v4f32_dot(v4f32 u, v4f32 v)
{
    f32 result = 0;
    for (u32 i = 0; i < 4; ++i) {
        result += u.c[i] * v.c[i];
    }
    return result;
}

//
// Matrix 4x4 functions
//

m4x4f32
m4x4f32_identity()
{
    return (m4x4f32){{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        }};
}
