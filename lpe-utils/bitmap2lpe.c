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
    type = LIBLPE_TYPE_RGB24;
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
  
  size_t input_size = liblpe_get_bitmap_size(&image_info);
  uint8_t* input = malloc(input_size);
  
  size_t bytes_read = 0;
  while (bytes_read != input_size) {
    if (feof(stdin)) {
      fputs("bitmap2lpe: Unexpected end of file\n", stderr);
      return 1;
    }
    
    bytes_read += fread(input+bytes_read, 1, input_size-bytes_read, stdin);
  }
  
  size_t output_size = liblpe_get_compressed_size(&image_info);
  uint8_t* output = malloc(output_size);
  
  liblpe_status_t result = liblpe_encode(&image_info, input, input_size, output);
  
  if (result) {
    fprintf(stderr, "bitmap2lpe: %s\n", liblpe_status_to_string(result));
    return 1;
  }
  
  fwrite(output, output_size, 1, stdout);
  
  if (output_size < input_size) {
    fprintf(stderr, "%zu -> %zu (about %zu%% smaller)\n", input_size, output_size, 100-(output_size*100/input_size));
  } else {
    fprintf(stderr, "%zu -> %zu\n", input_size, output_size);
  }
  
  return 0;
}
