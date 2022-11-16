#ifndef _FINCH_UTILS_H
#define _FINCH_UTILS_H

#include "finch/core.h"

#define UNUSED(x) ((void)x)

void swap_char(char* a, char* b);
u32 clamp_u32(u32 value_to_clamp, u32 min, u32 max);
u32 read_entire_file(char* dest, u32 max_bytes, char* filepath);

#endif // FINCH_UTILS_H
