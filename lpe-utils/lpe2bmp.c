#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <liblpe.h>

#define WRITE_U16(a) \
  putchar(a); \
  putchar(a >> 8);

#define WRITE_U32(a) \
  putchar(a); \
  putchar(a >> 8); \
  putchar(a >> 16); \
  putchar(a >> 24);

void output_bmp_file(liblpe_image_info_t* image_info, uint8_t* input, size_t input_size) {
  unsigned width_units = image_info->width / 4;
  unsigned width_alignment = image_info->width % 4;
  
  if (width_alignment) {
    width_units++;
  }
  
  unsigned width_aligned = width_units * 4;
  
  uint8_t width_alignment_buffer[width_alignment];
  
  for (unsigned i = 0; i < width_alignment; i++) {
    width_alignment_buffer[i] = 0;
  }
  
  uint32_t size = 54 + input_size;
  uint32_t offset = 54;
  uint32_t header_size = 40;
  uint32_t width = image_info->width;
  uint32_t height = -(int32_t)image_info->height;
  uint16_t planes = 1;
  uint16_t bit_count = 24;
  uint32_t compression = 0;
  uint32_t image_size = width_aligned * image_info->height * 3;
  uint32_t x_ppm = 3780;
  uint32_t y_ppm = 3780;
  uint32_t colors_used = 0;
  uint32_t colors_important = 0;
  
  fwrite("BM", 2, 1, stdout);
  WRITE_U32(size)
  WRITE_U32(0)
  WRITE_U32(offset)
  
  WRITE_U32(header_size)
  WRITE_U32(width)
  WRITE_U32(height)
  WRITE_U16(planes)
  WRITE_U16(bit_count)
  WRITE_U32(compression)
  WRITE_U32(image_size)
  WRITE_U32(y_ppm)
  WRITE_U32(x_ppm)
  WRITE_U32(colors_used)
  WRITE_U32(colors_important)
  
  if (image_info->type == LIBLPE_TYPE_GRAYSCALE) {
    for (unsigned y = 0; y < image_info->height; y++) {
      for (unsigned x = 0; x < image_info->width; x++) {
        putchar(input[y*image_info->width+x]);
        putchar(input[y*image_info->width+x]);
        putchar(input[y*image_info->width+x]);
      }
      
      fwrite(width_alignment_buffer, width_alignment, 1, stdout);
    }
  } else if (image_info->type == LIBLPE_TYPE_RGB24) {
    for (unsigned y = 0; y < image_info->height; y++) {
      for (unsigned x = 0; x < image_info->width; x++) {
        putchar(input[(y*image_info->width+x)*3+2]);
        putchar(input[(y*image_info->width+x)*3+1]);
        putchar(input[(y*image_info->width+x)*3+0]);
      }
      
      fwrite(width_alignment_buffer, width_alignment, 1, stdout);
    }
  }
}

int main(int argc, char* argv[]) {
  uint8_t* input = malloc(65536);
  size_t capacity = 65536;
  size_t bytes_read = 0;
  
  while (!feof(stdin)) {
    bytes_read += fread(input+bytes_read, 1, capacity-bytes_read, stdin);
    
    if (capacity == bytes_read) {
      capacity *= 2;
      input = realloc(input, capacity);
    }
  }
  
  liblpe_image_info_t image_info;
  
  liblpe_status_t image_info_result = liblpe_decode_image_info(input, bytes_read, &image_info);
  
  if (image_info_result) {
    fprintf(stderr, "lpe2bmp: %s\n", liblpe_status_to_string(image_info_result));
    return 1;
  }
  
  size_t output_size = liblpe_get_bitmap_size(&image_info);
  uint8_t* output = malloc(output_size);
  
  liblpe_status_t result = liblpe_decode(input, bytes_read, output);
  
  if (result) {
    fprintf(stderr, "lpe2bmp: %s\n", liblpe_status_to_string(result));
    return 1;
  }
  
  output_bmp_file(&image_info, output, output_size);
  
  return 0;
}
