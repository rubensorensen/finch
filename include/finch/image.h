#ifndef _FINCH_IMAGE_H
#define _FINCH_IMAGE_H

#include "finch/core.h"

// Currently we only support a subset of the TGA specification.
// Specifically, we support uncompressed color mapped images,
// uncompressed true color images and run-length encoded true
// color images. In the future we plan to support the full TGA
// specification as well as other formats, such as PNG, JPEG and BMP


typedef enum _TgaColorMapType {
    TGA_COLOR_MAP_TYPE_NO_COLOR_MAP = 0,
    TGA_COLOR_MAP_TYPE_COLOR_MAP = 1,
} TgaColorMapType;

typedef enum _TgaImageType {
    TGA_IMAGE_TYPE_NO_IMAGE_DATA = 0,
    TGA_IMAGE_TYPE_UNCOMPRESSED_COLOR_MAPPED_IMAGE = 1,
    TGA_IMAGE_TYPE_UNCOMPRESSED_TRUE_COLOR_IMAGE = 2,
    TGA_IMAGE_TYPE_UNCOMPRESSED_BLACK_AND_WHITE_IMAGE = 3,
    TGA_IMAGE_TYPE_RUN_LENGTH_ENCODED_COLOR_MAPPED_IMAGE = 9,
    TGA_IMAGE_TYPE_RUN_LENGTH_ENCODED_TRUE_COLOR_IMAGE = 10,
    TGA_IMAGE_TYPE_RUN_LENGTH_ENCODED_BLACK_AND_WHITE_IMAGE = 11,
} TgaImageType;

typedef struct _TgaColorMapSpecification {
    uint16_t first_entry_index;
    uint16_t color_map_length;
    u8  color_map_entry_size;
} TgaColorMapSpecification;

typedef struct _TgaImageSpecification {
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    u8 pixel_depth;
    u8 image_descriptor;
} TgaImageSpecification;

typedef struct _TgaHeader {
    u8 id_length;
    TgaColorMapType color_map_type;
    TgaImageType image_type;
    TgaColorMapSpecification color_map_specification;
    TgaImageSpecification image_specification;
} TgaHeader;

typedef struct _FcImage {
    u32 width;
    u32 height;
    u32* pixels;
} FcImage;

FcImage load_image_from_file(const char* filepath);

#endif // _FINCH_IMAGE_H
