#ifndef _FINCH_UTILS_H
#define _FINCH_UTILS_H

#include "finch/core.h"

#define UNUSED(x) ((void)x)
#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

void swap_char(char* a, char* b);
u32 clamp_u32(u32 value_to_clamp, u32 min, u32 max);
b32 slurp_file(const char* filepath, u8** dest, u32* bytes_read);

#endif // FINCH_UTILS_H
