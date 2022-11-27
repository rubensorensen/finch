#include "finch/utils.h"
#include "finch/log.h"

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
#include <stdlib.h>

b32
slurp_file(const char* filename, u8** dest, u32* bytes_read)
{
    FILE* f = fopen(filename, "rb");
    if (!f) {
        FC_ERROR("[ERROR] Could not open file %s: %s\n", filename, strerror(errno));
        return false;
    }
    
    fseek(f, 0L, SEEK_END);
    u32 file_size = ftell(f);
    fseek(f, 0L, SEEK_SET);

    *dest = (u8*)malloc(file_size * sizeof(u8));
    if (!*dest) {
        FC_ERROR("[ERROR] Could not allocate %u bytes: %s\n", file_size, strerror(errno));
        return false;
    }

    if (fread(*dest, 1, file_size, f) != file_size) {
        FC_ERROR("[ERROR] Could not read entire file for some reason\n");
        return false;
    }

    fclose(f);

    *bytes_read = file_size;
    
    return true;
}

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
