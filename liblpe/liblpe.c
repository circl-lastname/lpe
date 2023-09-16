#include <stddef.h>
#include <stdint.h>

#include "liblpe.h"

size_t liblpe_encode_get_output_size(liblpe_image_info_t* image_info) {
  unsigned x_blocks = image_info->width / 8;
  
  if (image_info->width % 8) {
    x_blocks++;
  }
  
  unsigned y_blocks = image_info->height / 8;
  
  if (image_info->height % 8) {
    y_blocks++;
  }
  
  unsigned blocks = x_blocks*y_blocks;
  
  if (image_info->type == LIBLPE_TYPE_RGB) {
    blocks *= 3;
  }
  
  return 8 + blocks * 10;
}

#define INPUT_AT_XY(x, y) input_u8[((y)*image_info->width+(x))*channels+channel]
#define BLOCK_AT_XY(x, y, o) output_u8[8+channel*blocks_per_channel*10+((y)*x_blocks+(x))*10+(o)]

void liblpe_encode(liblpe_image_info_t* image_info, const void* input, void* output) {
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
  
  output_u8[0] = 'L';
  output_u8[1] = 'P';
  output_u8[2] = 'E';
  
  output_u8[3] = 1;
  
  unsigned channels = 1;
  
  switch (image_info->type) {
    case LIBLPE_TYPE_GRAYSCALE:
    default:
      channels = 1;
      output_u8[3] |= LIBLPE_TYPE_GRAYSCALE << 4;
    break;
    case LIBLPE_TYPE_RGB:
      channels = 3;
      output_u8[3] |= LIBLPE_TYPE_RGB << 4;
    break;
  }
  
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
        
        uint8_t* block = &BLOCK_AT_XY(block_x, block_y, 0);
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
        if (y < image_info->height && raw_x < image_info->width) {
          unsigned x;
          
          if (y % 2) {
            x = (image_info->width-1) - raw_x;
          } else {
            x = raw_x;
          }
          
          uint8_t* block = &BLOCK_AT_XY(x/8, y/8, 0);
          
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
  }
}

#undef INPUT_AT_XY
#undef BLOCK_AT_XY

liblpe_image_info_t liblpe_decode_get_image_info(const void* input) {
  const uint8_t* input_u8 = input;
  
  liblpe_image_info_t image_info = { 0, 0, LIBLPE_TYPE_GRAYSCALE };
  
  if (input_u8[0] == 'L' && input_u8[1] == 'P' && input_u8[2] == 'E' && (input_u8[3] & 0x0f) == 1) {
    image_info.width |= input_u8[4];
    image_info.width |= input_u8[5] << 8;
    image_info.height |= input_u8[6];
    image_info.height |= input_u8[7] << 8;
    image_info.type = input_u8[3] >> 4;
  }
  
  return image_info;
}

size_t liblpe_decode_get_output_size(const void* input) {
  liblpe_image_info_t image_info = liblpe_decode_get_image_info(input);
  
  unsigned pixels = image_info.width*image_info.height;
  
  if (image_info.type == LIBLPE_TYPE_RGB) {
    pixels *= 3;
  }
  
  return pixels;
}

#define OUTPUT_AT_XY(x, y) output_u8[((y)*image_info->width+(x))*channels+channel]
#define BLOCK_AT_XY(x, y, o) input_u8[8+channel*blocks_per_channel*10+((y)*x_blocks+(x))*10+(o)]

void liblpe_decode(const void* input, void* output) {
  liblpe_image_info_t image_info_raw = liblpe_decode_get_image_info(input);
  liblpe_image_info_t* image_info = &image_info_raw;
  
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
  
  unsigned channels = 1;
  
  switch (image_info->type) {
    case LIBLPE_TYPE_GRAYSCALE:
    default:
      channels = 1;
    break;
    case LIBLPE_TYPE_RGB:
      channels = 3;
    break;
  }
  
  for (unsigned channel = 0; channel < channels; channel++) {
    for (unsigned block_y = 0; block_y < y_blocks; block_y++) {
      for (unsigned block_x = 0; block_x < x_blocks; block_x++) {
        const uint8_t* block = &BLOCK_AT_XY(block_x, block_y, 0);
        uint8_t low = *(block+0);
        uint8_t high = *(block+1);
        
        for (unsigned y = 0; y < 8; y++) {
          for (unsigned x = 0; x < 8; x++) {
            if (block_y*8+y < image_info->height && block_x*8+x < image_info->width) {
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
}
