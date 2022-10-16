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

// TODO: Remove libc dependency (Use platform-specific functions for reading files)
#include <stdio.h>
#include <string.h>
#include <errno.h>
u32 read_entire_file(char* dest, u32 max_bytes, char* filepath)
{
    FILE* file = fopen(filepath, "rb");
    if (file == NULL) {
        FC_ERROR("Could not open file %s: %s", filepath, strerror(errno));
        return 0;
    }

    fseek(file, 0, SEEK_END);          // Seek to end of file
    u32 file_size_bytes = ftell(file); // Get current file pointer
    fseek(file, 0, SEEK_SET);          // Seek back to beginning of file

    if (file_size_bytes > max_bytes) {
        FC_ERROR("File %s is larger than the allowed max_size (File size is %u bytes, max allowed size is %u bytes)",
                 filepath, file_size_bytes, max_bytes);
        return 0;
    }
    
    
    fread(dest, file_size_bytes, 1, file);
    fclose(file);

    return file_size_bytes;
}
