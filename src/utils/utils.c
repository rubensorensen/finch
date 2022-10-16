#include "finch/utils/utils.h"
#include "finch/log/log.h"

#include <math.h>

void swap_char(char* a, char* b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

u32 clamp_u32(u32 value_to_clamp, u32 min, u32 max)
{
    if (value_to_clamp < min) {
        value_to_clamp = min;
    }

    if (value_to_clamp > max) {
        value_to_clamp = max;
    }

    return value_to_clamp;
}
