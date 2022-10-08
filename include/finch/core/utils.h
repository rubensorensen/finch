#ifndef FINCH_CORE_UTILS_H
#define FINCH_CORE_UTILS_H

#include "finch/core/core.h"

void swap_char(char* a, char* b);
u32 string_length_null_terminated(char* str);
void reverse_string(char* str, u32 length);
void reverse_string_null_terminated(char* str);
u32 s64_to_null_terminated_string(s64 number, char* buf, u32 buf_size, u16 base);
u32 u64_to_null_terminated_string(u64 number, char* buf, u32 buf_size, u16 base);
char* string_copy(char* dest, char* src);
char* f64_to_null_terminated_string(f64 number, char* buf, u32 buf_size, u32 num_decimals);
    
#endif // FINCH_CORE_UTILS_H
