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
m4f32_print(m4f32 m)
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
    return sqrt(result);
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
    return sqrt(result);
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
    return sqrt(result);
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

m4f32
m4f32_identity()
{
    return (m4f32){{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        }};
}

// Transformation

m4f32
m4f32_translate(m4f32 m, v3f32 v)
{
    m4f32 result = m;
    result.cols[3] =
        v4f32_add_v4f32(
            v4f32_add_v4f32(
                v4f32_add_v4f32(
                    v4f32_mul_f32(m.cols[0], v.c[0]),
                    v4f32_mul_f32(m.cols[1], v.c[1])),
                v4f32_mul_f32(m.cols[2], v.c[2])),
            m.cols[3]);
    return result;
}

m4f32
m4f32_rotate(m4f32 matrix, f32 angle, v3f32 axis)
{
	f32 s = sin(angle);
	f32 c = cos(angle);

    axis = v3f32_normalize(axis);
    v3f32 temp = v3f32_mul_f32(axis, 1.0f - c);

    m4f32 rotate = {0};
    rotate.cols[0].c[0] = c + temp.c[0] * axis.c[0];
    rotate.cols[0].c[1] = temp.c[0] * axis.c[1] + s * axis.c[2];
    rotate.cols[0].c[2] = temp.c[0] * axis.c[2] - s * axis.c[1];

    rotate.cols[1].c[0] = temp.c[1] * axis.c[0] - s * axis.c[2];
    rotate.cols[1].c[1] = c + temp.c[1] * axis.c[1];
    rotate.cols[1].c[2] = temp.c[1] * axis.c[2] + s * axis.c[0];

    rotate.cols[2].c[0] = temp.c[2] * axis.c[0] + s * axis.c[1];
    rotate.cols[2].c[1] = temp.c[2] * axis.c[1] - s * axis.c[0];
    rotate.cols[2].c[2] = c + temp.c[2] * axis.c[2];

    m4f32 result;
    result.cols[0] = v4f32_add_v4f32(v4f32_add_v4f32(
                                         v4f32_mul_f32(matrix.cols[0], rotate.cols[0].c[0]),
                                         v4f32_mul_f32(matrix.cols[1], rotate.cols[0].c[1])),
                                     v4f32_mul_f32(matrix.cols[2], rotate.cols[0].c[2]));
    
    result.cols[1] = v4f32_add_v4f32(v4f32_add_v4f32(
                                         v4f32_mul_f32(matrix.cols[0], rotate.cols[1].c[0]),
                                         v4f32_mul_f32(matrix.cols[1], rotate.cols[1].c[1])),
                                     v4f32_mul_f32(matrix.cols[2], rotate.cols[1].c[2]));
    
    result.cols[2] = v4f32_add_v4f32(v4f32_add_v4f32(
                                         v4f32_mul_f32(matrix.cols[0], rotate.cols[2].c[0]),
                                         v4f32_mul_f32(matrix.cols[1], rotate.cols[2].c[1])),
                                     v4f32_mul_f32(matrix.cols[2], rotate.cols[2].c[2]));
    result.cols[3] = matrix.cols[3];

    return result;
}

m4f32
m4f32_scale(m4f32 m, v3f32 v)
{
    m4f32 result = {0};
    result.cols[0] = v4f32_mul_f32(m.cols[0], v.c[0]);
    result.cols[1] = v4f32_mul_f32(m.cols[1], v.c[1]);
    result.cols[2] = v4f32_mul_f32(m.cols[2], v.c[2]);
    result.cols[3] = m.cols[3];
    return result;
}

m4f32
m4f32_look_at(v3f32 eye, v3f32 center, v3f32 up)
{
    v3f32 f = v3f32_normalize(v3f32_sub_v3f32(center, eye));
    v3f32 s = v3f32_normalize(v3f32_cross(f, up));
    v3f32 u = v3f32_cross(s, f);

    m4f32 result = m4f32_identity();
    result.cols[0].c[0] =  s.x;
    result.cols[1].c[0] =  s.y;
    result.cols[2].c[0] =  s.z;
    result.cols[0].c[1] =  u.x;
    result.cols[1].c[1] =  u.y;
    result.cols[2].c[1] =  u.z;
    result.cols[0].c[2] = -f.x;
    result.cols[1].c[2] = -f.y;
    result.cols[2].c[2] = -f.z;
    result.cols[3].c[0] = -v3f32_dot(s, eye);
    result.cols[3].c[1] = -v3f32_dot(u, eye);
    result.cols[3].c[2] =  v3f32_dot(f, eye);
    
    return result;
}

m4f32
m4f32_perspective(f32 fovy, f32 aspect_ratio, f32 z_near, f32 z_far)
{
    
    f32 tan_half_fovy = tan(fovy / 2.0f);

    m4f32 result = {0};
    result.cols[0].c[0] = 1.0f / (aspect_ratio * tan_half_fovy);
    result.cols[1].c[1] = 1.0f / tan_half_fovy;
    result.cols[2].c[2] = z_far / (z_near - z_far);
    result.cols[2].c[3] = -1.0f;
    result.cols[3].c[2] = -(z_far * z_near) / (z_far - z_near);    
    return result;
}
