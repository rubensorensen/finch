#include "finch/utils/string.h"
#include "finch/utils/utils.h"
#include "finch/log/log.h"
#include "finch/platform/platform.h"

#include <stdlib.h>

u32 string_length_null_terminated(char* str)
{
    u32 length = 0;
    while (str && *str != '\0') {
        length += 1;
        str += 1;
    }
    return length;
}

void string_reverse(char* str, u32 length)
{
    for (u32 i = 0; i < length / 2; ++i) {
        swap_char((str + i), (str + length - i - 1));
    }
}

void string_reverse_null_terminated(char* str)
{
    u32 length = string_length_null_terminated(str);
    string_reverse(str, length);
}

u32 s64_to_string_null_terminated(s64 number, char* buf, u32 buf_size, u16 base)
{
    if (buf_size < 2) {
        FC_ENGINE_ERROR("Buffer size must be at least 2");
        exit(EXIT_FAILURE);
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
    string_reverse_null_terminated(buf);
 
    return length;
}

u32 u64_to_string_null_terminated(u64 number, char* buf, u32 buf_size, u16 base)
{
    if (buf_size < 2) {
        FC_ENGINE_ERROR("Buffer size must be at least 2");
        exit(EXIT_FAILURE);
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
    string_reverse_null_terminated(buf);
 
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


// TODO: Remove dependency on libc math
#include "math.h"
char* f64_to_string_null_terminated(f64 number, char* buf, u32 buf_size, u32 num_decimals) {
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


u32 string_format(char* dest, u32 max_size, const char* fmt, ...)
{
    char temp[1024] = {0};
    u32 i = 0, j = 0;

    va_list args;
    va_start(args, fmt);
        
    while (fmt && fmt[i] && j < max_size - 1) {

        // If character is not a '%', treat no formatting is necessary
        if (fmt[i] != '%') {
            dest[j++] = fmt[i++];
            continue;
        }
    
        // Character is '%', formatting is necessary
        i += 1;
        switch (fmt[i]) {
            case '%': {
                dest[j++] = fmt[i++];
            } break;
            case 'd': {
                s64_to_string_null_terminated(va_arg(args, s32),
                                              temp, 1024, 10);
            } break;
            case 'u': {
                u64_to_string_null_terminated(va_arg(args, u32),
                                              temp, 1024, 10);
            } break;
            case 'x': {
                u64_to_string_null_terminated(va_arg(args, u32),
                                              temp, 1024, 16);
            } break;
            case 'b': {
                u64_to_string_null_terminated(va_arg(args, u32),
                                              temp, 1024, 2);
            } break;
            case 'f': {
                f64_to_string_null_terminated(va_arg(args, f64),
                                              temp, 1024, 5);
            } break;
            case 'c': {
                temp[0] = (char)va_arg(args, s32);
                temp[1] = '\0';
            } break;
            case 's': {
                string_copy(temp, va_arg(args, char*));
            } break;
            case 'p': {
                temp[0] = '0';
                temp[1] = 'x';
                u64_to_string_null_terminated(va_arg(args, u64),
                                              temp + 2, 1024, 16);
            } break;
            default: {
                string_format(temp, 1024,
                              "Logger: Unrecognized format option: '%c'\n", fmt[i]);
                platform_write_to_stderr(temp);
                temp[0] = '?';
                temp[1] = '\0';
            }
        }
        
        string_copy(dest + j, temp);
        j += string_length_null_terminated(temp);
        i += 1;
    }

    dest[j] = '\0';
    va_end(args);

    return j;
}
