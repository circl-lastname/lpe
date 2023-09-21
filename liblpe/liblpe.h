#pragma once

#include <stddef.h>
#include <stdint.h>

typedef enum {
  LIBLPE_STATUS_SUCCESS,
  LIBLPE_STATUS_TOO_SMALL,
  LIBLPE_STATUS_INVALID_TYPE,
  LIBLPE_STATUS_NOT_LPE,
  LIBLPE_STATUS_TOO_NEW,
  LIBLPE_STATUS_LAST,
} liblpe_status_t;

typedef enum {
  LIBLPE_TYPE_GRAYSCALE = 0x1,
  LIBLPE_TYPE_RGB24 = 0x3,
} liblpe_type_t;

typedef struct {
  uint16_t width;
  uint16_t height;
  liblpe_type_t type;
} liblpe_image_info_t;

size_t liblpe_get_bitmap_size(liblpe_image_info_t* image_info);
size_t liblpe_get_compressed_size(liblpe_image_info_t* image_info);

const char* liblpe_status_to_string(liblpe_status_t status);

liblpe_status_t liblpe_encode(liblpe_image_info_t* image_info, const void* input, size_t input_size, void* output);

liblpe_status_t liblpe_decode_image_info(const void* input, size_t input_size, liblpe_image_info_t* image_info);
liblpe_status_t liblpe_decode(const void* input, size_t input_size, void* output);
