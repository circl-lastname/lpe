#include <stddef.h>
#include <stdint.h>

#include "liblpe.h"

const char* status_strings[] = {
  [LIBLPE_STATUS_SUCCESS] = "Success",
  [LIBLPE_STATUS_TOO_SMALL] = "File is too small to fit the full image",
  [LIBLPE_STATUS_INVALID_TYPE] = "Image has invalid or unsupported type",
  [LIBLPE_STATUS_NOT_LPE] = "File is not an LPE file or its header is corrupted",
  [LIBLPE_STATUS_TOO_NEW] = "The version of the LPE file is too new (max version 1)",
};

size_t liblpe_get_bitmap_size(liblpe_image_info_t* image_info) {
  unsigned channels = 0;
  
  switch (image_info->type) {
    case LIBLPE_TYPE_GRAYSCALE:
      channels = 1;
    break;
    case LIBLPE_TYPE_RGB24:
      channels = 3;
    break;
    default:
      return 0;
  }
  
  return image_info->width * image_info->height * channels;
}

size_t liblpe_get_compressed_size(liblpe_image_info_t* image_info) {
  unsigned channels = 0;
  
  switch (image_info->type) {
    case LIBLPE_TYPE_GRAYSCALE:
      channels = 1;
    break;
    case LIBLPE_TYPE_RGB24:
      channels = 3;
    break;
    default:
      return 0;
  }
  
  unsigned x_blocks = image_info->width / 8;
  
  if (image_info->width % 8) {
    x_blocks++;
  }
  
  unsigned y_blocks = image_info->height / 8;
  
  if (image_info->height % 8) {
    y_blocks++;
  }
  
  return 8 + x_blocks * y_blocks * channels * 10;
}

const char* liblpe_status_to_string(liblpe_status_t status) {
  if (status >= 0 && status < LIBLPE_STATUS_LAST) {
    return status_strings[status];
  } else {
    return "Unknown error";
  }
}

#define INPUT_AT_XY(x, y) input_u8[((y)*image_info->width+(x))*channels+channel]
#define BLOCK_AT_XY(x, y) output_u8[8+channel*blocks_per_channel*10+((y)*x_blocks+(x))*10]

liblpe_status_t liblpe_encode(liblpe_image_info_t* image_info, const void* input, size_t input_size, void* output) {
  if (input_size < liblpe_get_bitmap_size(image_info)) {
    return LIBLPE_STATUS_TOO_SMALL;
  }
  
  const uint8_t* input_u8 = input;
  uint8_t* output_u8 = output;
  
  unsigned x_blocks = image_info->width / 8;
  
  if (image_info->width % 8) {
    x_blocks++;
  }
  
  unsigned y_blocks = image_info->height / 8;
  
  if (image_info->height % 8) {
    y_blocks++;
  }
  
  unsigned blocks_per_channel = x_blocks*y_blocks;
  
  unsigned channels = 0;
  
  switch (image_info->type) {
    case LIBLPE_TYPE_GRAYSCALE:
      channels = 1;
    break;
    case LIBLPE_TYPE_RGB24:
      channels = 3;
    break;
    default:
      return LIBLPE_STATUS_INVALID_TYPE;
  }
  
  output_u8[0] = 'L';
  output_u8[1] = 'P';
  output_u8[2] = 'E';
  
  output_u8[3] = 1;
  output_u8[3] |= image_info->type << 4;
  
  output_u8[4] = image_info->width;
  output_u8[5] = image_info->width >> 8;
  output_u8[6] = image_info->height;
  output_u8[7] = image_info->height >> 8;
  
  for (unsigned channel = 0; channel < channels; channel++) {
    for (unsigned block_y = 0; block_y < y_blocks; block_y++) {
      for (unsigned block_x = 0; block_x < x_blocks; block_x++) {
        int low = 256;
        int high = -1;
        
        for (unsigned y = 0; y < 8; y++) {
          for (unsigned x = 0; x < 8; x++) {
            if (block_y*8+y < image_info->height && block_x*8+x < image_info->width) {
              uint8_t value = INPUT_AT_XY(block_x*8+x, block_y*8+y);
              
              if (value < low) {
                low = value;
              }
              
              if (value > high) {
                high = value;
              }
            }
          }
        }
        
        uint8_t* block = &BLOCK_AT_XY(block_x, block_y);
        *(block+0) = low;
        *(block+1) = high;
        *(block+2) = 0;
        *(block+3) = 0;
        *(block+4) = 0;
        *(block+5) = 0;
        *(block+6) = 0;
        *(block+7) = 0;
        *(block+8) = 0;
        *(block+9) = 0;
      }
    }
    
    for (unsigned y = 0; y < image_info->height; y++) {
      int error = 0;
      
      for (unsigned raw_x = 0; raw_x < image_info->width; raw_x++) {
        unsigned x;
        
        if (y % 2) {
          x = (image_info->width-1) - raw_x;
        } else {
          x = raw_x;
        }
        
        uint8_t* block = &BLOCK_AT_XY(x/8, y/8);
        
        int target = INPUT_AT_XY(x, y) + error;
        
        int delta_low = target - *(block+0);
        int delta_high = target - *(block+1);
        
        delta_low = delta_low < 0 ? -delta_low : delta_low;
        delta_high = delta_high < 0 ? -delta_high : delta_high;
        
        int quantized = 0;
        unsigned value = 0;
        
        if (delta_low <= delta_high) {
          quantized = *(block+0);
          value = 0;
        } else {
          quantized = *(block+1);
          value = 1;
        }
        
        *(block+(2 + y % 8)) |= value << (x % 8);
        
        error = target - quantized;
      }
    }
  }
  
  return LIBLPE_STATUS_SUCCESS;
}

#undef INPUT_AT_XY
#undef BLOCK_AT_XY

liblpe_status_t liblpe_decode_image_info(const void* input, size_t input_size, liblpe_image_info_t* image_info) {
  const uint8_t* input_u8 = input;
  
  if (input_size < 8) {
    return LIBLPE_STATUS_NOT_LPE;
  }
  
  if (!(input_u8[0] == 'L' && input_u8[1] == 'P' && input_u8[2] == 'E')) {
    return LIBLPE_STATUS_NOT_LPE;
  }
  
  if ((input_u8[3] & 0x0f) != 1) {
    return LIBLPE_STATUS_TOO_NEW;
  }
  
  image_info->width = 0;
  image_info->width |= input_u8[4];
  image_info->width |= input_u8[5] << 8;
  image_info->height = 0;
  image_info->height |= input_u8[6];
  image_info->height |= input_u8[7] << 8;
  image_info->type = input_u8[3] >> 4;
  
  switch (image_info->type) {
    case LIBLPE_TYPE_GRAYSCALE:
    case LIBLPE_TYPE_RGB24:
    break;
    default:
      return LIBLPE_STATUS_INVALID_TYPE;
  }
  
  return LIBLPE_STATUS_SUCCESS;
}

#define OUTPUT_AT_XY(x, y) output_u8[((y)*image_info.width+(x))*channels+channel]
#define BLOCK_AT_XY(x, y) input_u8[8+channel*blocks_per_channel*10+((y)*x_blocks+(x))*10]

liblpe_status_t liblpe_decode(const void* input, size_t input_size, void* output) {
  liblpe_image_info_t image_info;
  
  liblpe_status_t header_decode_status = liblpe_decode_image_info(input, input_size, &image_info);
  
  if (header_decode_status) {
    return header_decode_status;
  }
  
  if (input_size < liblpe_get_compressed_size(&image_info)) {
    return LIBLPE_STATUS_TOO_SMALL;
  }
  
  const uint8_t* input_u8 = input;
  uint8_t* output_u8 = output;
  
  unsigned x_blocks = image_info.width / 8;
  
  if (image_info.width % 8) {
    x_blocks++;
  }
  
  unsigned y_blocks = image_info.height / 8;
  
  if (image_info.height % 8) {
    y_blocks++;
  }
  
  unsigned blocks_per_channel = x_blocks*y_blocks;
  
  unsigned channels = image_info.type;
  
  for (unsigned channel = 0; channel < channels; channel++) {
    for (unsigned block_y = 0; block_y < y_blocks; block_y++) {
      for (unsigned block_x = 0; block_x < x_blocks; block_x++) {
        const uint8_t* block = &BLOCK_AT_XY(block_x, block_y);
        uint8_t low = *(block+0);
        uint8_t high = *(block+1);
        
        for (unsigned y = 0; y < 8; y++) {
          for (unsigned x = 0; x < 8; x++) {
            if (block_y*8+y < image_info.height && block_x*8+x < image_info.width) {
              if (*(block+(2 + y)) & (1 << x)) {
                OUTPUT_AT_XY(block_x*8+x, block_y*8+y) = high;
              } else {
                OUTPUT_AT_XY(block_x*8+x, block_y*8+y) = low;
              }
            }
          }
        }
      }
    }
  }
  
  return LIBLPE_STATUS_SUCCESS;
}
