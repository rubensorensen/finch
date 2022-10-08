#include "finch/core/utils.h"
#include "finch/log/log.h"

#include <math.h>

void swap_char(char* a, char* b)
{
    char temp = *a;
    *a = *b;
    *b = temp;
}

u32 string_length_null_terminated(char* str)
{
    u32 length = 0;
    while (str && *str != '\0') {
        length += 1;
        str += 1;
    }
    return length;
}

void reverse_string(char* str, u32 length)
{
    for (u32 i = 0; i < length / 2; ++i) {
        swap_char((str + i), (str + length - i - 1));
    }
}

void reverse_string_null_terminated(char* str)
{
    u32 length = string_length_null_terminated(str);
    reverse_string(str, length);
}

u32 s64_to_null_terminated_string(s64 number, char* buf, u32 buf_size, u16 base)
{
    if (buf_size < 2) {
        FC_ENGINE_FATAL("Buffer size must be at least 2");
    }
    
    u32 length = 0;
    b32 is_negative = false;
 
    if (number == 0) {
        buf[length++] = '0';
        buf[length] = '\0';
        return length;
    }
 
    if (number < 0 && base == 10) {
        is_negative = true;
        number *= -1;
    }
 
    while (number != 0 && length < buf_size - 1) {
        u64 rem = number % base;
        buf[length++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        number /= base;
    }
 
    if (is_negative && length < buf_size - 1) {
        buf[length++] = '-';
    }
 
    buf[length] = '\0';
    reverse_string_null_terminated(buf);
 
    return length;
}

u32 u64_to_null_terminated_string(u64 number, char* buf, u32 buf_size, u16 base)
{
    if (buf_size < 2) {
        FC_ENGINE_FATAL("Buffer size must be at least 2");
    }
    
    u32 length = 0;
 
    if (number == 0) {
        buf[length++] = '0';
        buf[length] = '\0';
        return length;
    }
 
    while (number != 0 && length < buf_size - 1) {
        u64 rem = number % base;
        buf[length++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        number /= base;
    }
 
    buf[length] = '\0';
    reverse_string_null_terminated(buf);
 
    return length;
}

char* string_copy(char* dest, char* src)
{
    char* ptr = dest;
    if (ptr == 0) {
        return 0;
    }
    
    while (src && *src != '\0') {
        *dest++ = *src++;
    }
    *dest = '\0';

    return ptr;
}

char* f64_to_null_terminated_string(f64 number, char* buf, u32 buf_size, u32 num_decimals) {
    char* s = buf + buf_size;
    *--s = '\0';
    s32 k = pow(10, num_decimals);
    u16 decimals;
    s32 units;
    if (number < 0) {
        decimals = (s32)(number * -k) % k;
        units = (s32)(number * -1);
    } else {
        decimals = (s32)(number * k) % k;
        units = (s32)number;
    }

    for (u32 i = 0; i < num_decimals; ++i) {
        *--s = (decimals % 10) + '0';
        decimals /= 10;
    }
    *--s = '.';

    while (units > 0) {
        *--s = (units % 10) + '0';
        units /= 10;
    }
    if (number < 0) *--s = '-';

    string_copy(buf, s);
    
    return buf;
}
