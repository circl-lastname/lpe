#include <liblpe.h>

void print_int(int arg);
int get_lpe_size();
int get_lpe_byte();

uint8_t lpe_file[655368];

uint8_t bitmap[2048*2048*3];
int bitmap_index;
size_t bitmap_size;
int bitmap_width;
int bitmap_height;

int decode_lpe() {
  size_t lpe_size = get_lpe_size();
  
  if (lpe_size > 655368) {
    return 0;
  }
  
  for (unsigned i = 0; i < lpe_size; i++) {
    lpe_file[i] = get_lpe_byte();
  }
  
  liblpe_image_info_t image_info;
  liblpe_status_t image_info_result = liblpe_decode_image_info(lpe_file, lpe_size, &image_info);
  
  if (image_info_result) {
    return 0;
  }
  
  bitmap_size = liblpe_get_bitmap_size(&image_info);
  
  if (bitmap_size > 2048*2048*3 || image_info.type != LIBLPE_TYPE_RGB24) {
    return 0;
  }
  
  liblpe_status_t result = liblpe_decode(lpe_file, lpe_size, bitmap);
  
  if (result) {
    return 0;
  }
  
  bitmap_index = 0;
  bitmap_width = image_info.width;
  bitmap_height = image_info.height;
  
  return 1;
}

int get_bitmap_size() {
  return bitmap_size;
}

int get_bitmap_width() {
  return bitmap_width;
}

int get_bitmap_height() {
  return bitmap_height;
}

int get_bitmap_byte() {
  return bitmap[bitmap_index++];
}
