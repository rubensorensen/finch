#include "finch/utils/utils.h"
#include "finch/log/log.h"

#include <math.h>

void swap_char(char* a, char* b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}
