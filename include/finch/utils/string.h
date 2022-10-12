#ifndef FINCH_UTILS_STRING_H
#define FINCH_UTILS_STRING_H

#include "finch/core/core.h"

u32 string_length_null_terminated(char* str);
void string_reverse(char* str, u32 length);
void string_reverse_null_terminated(char* str);
u32 s64_to_string_null_terminated(s64 number, char* buf, u32 buf_size, u16 base);
u32 u64_to_string_null_terminated(u64 number, char* buf, u32 buf_size, u16 base);
char* string_copy(char* dest, char* src);
char* f64_to_string_null_terminated(f64 number, char* buf, u32 buf_size, u32 num_decimals);

#endif // FINCH_UTILS_STRING_H
