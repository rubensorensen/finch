#include "finch/image.h"
#include "finch/utils.h"

#include <assert.h>
#include <stdlib.h>

static u8*
read_tga_header(u8* data, TgaHeader* header)
{
    header->id_length = *data++;
    header->color_map_type = (TgaColorMapType)*data++;
    header->image_type = (TgaImageType)*data++;

    TgaColorMapSpecification cms = {0};
    cms.first_entry_index = *(uint16_t*)data;
    data += 2;
    cms.color_map_length = *(uint16_t*)data;
    data += 2;
    cms.color_map_entry_size = *data++;
    header->color_map_specification = cms;
    
    TgaImageSpecification is = {0};
    is.x_origin = *(uint16_t*)data;
    data += 2;
    is.y_origin = *(uint16_t*)data;
    data += 2;
    is.width = *(uint16_t*)data;
    data += 2;
    is.height = *(uint16_t*)data;
    data += 2;
    is.pixel_depth = *data++;
    is.image_descriptor = *data++;
    header->image_specification = is;
    
    return data;
}


static FcImage
parse_tga_image(u8* data)
{
    FcImage image = {0};
    
    TgaHeader tga_header = {0};
    data = read_tga_header(data, &tga_header);
    
    assert(((tga_header.image_type == TGA_IMAGE_TYPE_UNCOMPRESSED_TRUE_COLOR_IMAGE) ||
            (tga_header.image_type == TGA_IMAGE_TYPE_RUN_LENGTH_ENCODED_TRUE_COLOR_IMAGE) ||
            (tga_header.image_type == TGA_IMAGE_TYPE_UNCOMPRESSED_COLOR_MAPPED_IMAGE))
           && "Unsupported image type");
    
    image.width = tga_header.image_specification.width;
    image.height = tga_header.image_specification.height;

    u8* image_id = data;
    data += tga_header.id_length;
    (void)image_id;

    u8* color_map_data = data;
    assert(((tga_header.color_map_specification.color_map_entry_size % 8) == 0) &&
           "Color map entry size in bits must be divisible by 8");
    u32 color_map_stride = tga_header.color_map_specification.color_map_entry_size / 8;
    data += tga_header.color_map_specification.color_map_length * color_map_stride;

    bool has_alpha = (tga_header.image_specification.image_descriptor & 0xF) > 2;
    assert(((tga_header.image_specification.pixel_depth % 8) == 0) &&
           (tga_header.image_specification.pixel_depth <= 32));
    u32 stride = tga_header.image_specification.pixel_depth / 8;
    bool right_to_left = (tga_header.image_specification.image_descriptor >> 4) & 1;
    bool bottom_to_top = !((tga_header.image_specification.image_descriptor >> 5) & 1);

    image.pixels = (u32*)malloc(image.width * image.height * sizeof(u32));
    
    switch (tga_header.image_type) {
        case TGA_IMAGE_TYPE_UNCOMPRESSED_TRUE_COLOR_IMAGE: {
            for (u32 y = 0; y < image.height; ++y) {
                for (u32 x = 0; x < image.width; ++x) {
                    u8* pixel = data + stride*(x + y*image.width);
                    u8 b = pixel[0];
                    u8 g = pixel[1];
                    u8 r = pixel[2];
                    u8 a = has_alpha ? pixel[3] : 0xFF;
                    u32 ix = right_to_left ? image.width - x: x;
                    u32 iy = bottom_to_top ? image.height - y - 1 : y;
                    u32 i = ix + iy * image.width;
                    image.pixels[i] = ((a << 24) | (b << 16) | (g << 8) | r);
                }
            }
        } break;
        case TGA_IMAGE_TYPE_UNCOMPRESSED_COLOR_MAPPED_IMAGE: {
            for (u32 y = 0; y < image.height; ++y) {
                for (u32 x = 0; x < image.width; ++x) {
                    u32 pixel_index = *(data + stride*(x + y*image.width));
                    u32* pixel = (u32*)(color_map_data + color_map_stride*pixel_index);
                    u8 b = pixel[0];
                    u8 g = pixel[1];
                    u8 r = pixel[2];
                    u8 a = has_alpha ? pixel[3] : 0xFF;
                    u32 ix = right_to_left ? image.width - x: x;
                    u32 iy = bottom_to_top ? image.height - y - 1 : y;
                    u32 i = ix + iy * image.width;
                    image.pixels[i] = ((a << 24) | (b << 16) | (g << 8) | r);
                }
            }
        } break;
        case TGA_IMAGE_TYPE_RUN_LENGTH_ENCODED_TRUE_COLOR_IMAGE: {

            for (u32 i = 0; i < image.width * image.height;) {
                u8 b = *data++;
                bool rlp = b >> 7;
                u8 rep = (b & 127) + 1;
                if (rlp) {
                    u8 b = data[0];
                    u8 g = data[1];
                    u8 r = data[2];
                    u8 a = has_alpha ? data[3] : 0xFF;
                    for (u32 j = 0; j < rep; ++j) {
                        u32 x = i % image.width;
                        u32 y = i / image.width;
                        u32 ix = right_to_left ? image.width - x : x;
                        u32 iy = bottom_to_top ? image.height - y - 1: y;
                        u32 index = ix + iy * image.width;
                        image.pixels[index] = ((a << 24) | (b << 16) | (g << 8) | r);
                        i += 1;
                    }
                    data += stride;
                    continue;
                }
                for (u32 j = 0; j < rep; ++j) {
                    u8 b = data[0];
                    u8 g = data[1];
                    u8 r = data[2];
                    u8 a = has_alpha ? data[3] : 0xFF;
                    u32 x = i % image.width;
                    u32 y = i / image.width;
                    u32 ix = right_to_left ? image.width - x : x;
                    u32 iy = bottom_to_top ? image.height - y - 1: y;
                    u32 index = ix + iy * image.width;
                    image.pixels[index] = ((r << 24) | (g << 16) | (b << 8) | a);
                    i += 1;
                    data += stride;
                }
            }
            
        } break;
        default: {
            image.width = 0;
            image.height = 0;
            image.pixels = NULL;
        } break;
    }

    return image;
}

FcImage
load_image_from_file(const char* filepath)
{
    u32 file_size;
    u8* file_data;
    slurp_file(filepath, &file_data, &file_size);

    FcImage image = parse_tga_image(file_data);

    free(file_data);
    
    return image;
}
