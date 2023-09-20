#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <liblpe.h>

int main(int argc, char* argv[]) {
  if (argc < 4) {
    fputs("usage: bitmap2lpe <width> <height> <grayscale/rgb24>\n", stderr);
    return 1;
  }
  
  int width = atoi(argv[1]);
  int height = atoi(argv[2]);
  liblpe_type_t type;
  
  if (!strcmp(argv[3], "grayscale")) {
    type = LIBLPE_TYPE_GRAYSCALE;
  } else if (!strcmp(argv[3], "rgb24")) {
    type = LIBLPE_TYPE_RGB;
  } else {
    fputs("bitmap2lpe: Valid image types are grayscale or rgb24\n", stderr);
    return 1;
  }
  
  if (width > 65535) {
    fputs("bitmap2lpe: Maximum width is 65535\n", stderr);
    return 1;
  }
  
  if (height > 65535) {
    fputs("bitmap2lpe: Maximum height is 65535\n", stderr);
    return 1;
  }
  
  liblpe_image_info_t image_info = {
    .width = width,
    .height = height,
    .type = type,
  };
  
  size_t input_size = width * height;
  
  if (type == LIBLPE_TYPE_RGB) {
    input_size *= 3;
  }
  
  uint8_t* input = malloc(input_size);
  
  for (size_t i = 0; i < input_size; i++) {
    input[i] = getchar();
  }
  
  size_t output_size = liblpe_encode_get_output_size(&image_info);
  
  uint8_t* output = malloc(output_size);
  
  liblpe_encode(&image_info, input, output);
  
  fwrite(output, output_size, 1, stdout);
  
  fprintf(stderr, "%zu -> %zu (about %zu%% smaller)\n", input_size, output_size, 100-(output_size*100/input_size));
  
  return 0;
}
