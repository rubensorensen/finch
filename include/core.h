#ifndef FINCH_CORE_H
#define FINCH_CORE_H

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

typedef union _Color {
    u32 packed;
    struct {
        u8 r, g, b, a;
    };
} Color;

u32 format_color(Color col);

#endif // FINCH_CORE_H
