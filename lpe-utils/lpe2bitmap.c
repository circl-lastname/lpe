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
  
  liblpe_image_info_t image_info = liblpe_decode_get_image_info(input);
  
  char* type;
  
  if (image_info.type == LIBLPE_TYPE_GRAYSCALE) {
    type = "grayscale";
  } else if (image_info.type == LIBLPE_TYPE_RGB) {
    type = "rgb24";
  } else {
    type = "unknown";
  }
  
  fprintf(stderr, "Width: %u\nHeight: %u\nType: %s\n", image_info.width, image_info.height, type);
  
  size_t output_size = liblpe_decode_get_output_size(input);
  
  uint8_t* output = malloc(output_size);
  
  liblpe_decode(input, output);
  
  fwrite(output, output_size, 1, stdout);
  
  return 0;
}
