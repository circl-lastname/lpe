#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <liblpe.h>

int main(int argc, char* argv[]) {
  uint8_t* input = malloc(1024);
  size_t capacity = 1024;
  size_t i = 0;
  
  int ch;
  while ((ch = getchar()) != EOF) {
    input[i] = ch;
    i++;
    
    if (i >= capacity) {
      capacity *= 2;
      input = realloc(input, capacity);
    }
  }
  
  liblpe_image_info_t image_info;
  
  liblpe_status_t image_info_result = liblpe_decode_image_info(input, i, &image_info);
  
  if (image_info_result) {
    fprintf(stderr, "lpe2bitmap: %s\n", liblpe_status_to_string(image_info_result));
    return 1;
  }
  
  char* type;
  
  if (image_info.type == LIBLPE_TYPE_GRAYSCALE) {
    type = "grayscale";
  } else if (image_info.type == LIBLPE_TYPE_RGB24) {
    type = "rgb24";
  } else {
    type = "unknown";
  }
  
  size_t output_size = liblpe_get_bitmap_size(&image_info);
  uint8_t* output = malloc(output_size);
  
  liblpe_status_t result = liblpe_decode(input, i, output);
  
  if (result) {
    fprintf(stderr, "lpe2bitmap: %s\n", liblpe_status_to_string(result));
    return 1;
  }
  
  fwrite(output, output_size, 1, stdout);
  
  fprintf(stderr, "Width: %u\nHeight: %u\nType: %s\n", image_info.width, image_info.height, type);
  
  return 0;
}
