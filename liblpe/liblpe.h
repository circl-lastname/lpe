#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
  LIBLPE_TYPE_GRAYSCALE = 0x1,
  LIBLPE_TYPE_RGB = 0x3,
} liblpe_type_t;

typedef struct {
  uint16_t width;
  uint16_t height;
  liblpe_type_t type;
} liblpe_image_info_t;

size_t liblpe_encode_get_output_size(liblpe_image_info_t* image_info);
void liblpe_encode(liblpe_image_info_t* image_info, const void* input, void* output);

liblpe_image_info_t liblpe_decode_get_image_info(const void* input);
size_t liblpe_decode_get_output_size(const void* input);
void liblpe_decode(const void* input, void* output);
