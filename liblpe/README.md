# liblpe
liblpe is the reference implementation of the LPE lossy image compression format. It is statically linked, and very highly portable. It is however a bit crude and may not be suited for embedded systems with hardware functions that could speed up the process.

WARNING: Currently there are some obvious memory safety problems!

## Installation
With `gcc` and `ninja` installed:

```
./configure
ninja liblpe
sudo ninja install-liblpe
```

## Encoding an LPE file
liblpe can encode either a top-left origin 8-bit grayscale or top-left origin RGB24 bitmap into an LPE file stored in a buffer. The following C code is an example:

```c
#include <liblpe.h>

// ...

liblpe_image_info_t image_info = { // Create the image info struct describing the bitmap
  .width = 640, // The width in pixels of the bitmap
  .height = 480, // The height in pixels of the bitmap
  .type = LIBLPE_TYPE_RGB24, // The type of bitmap, this can also be LIBLPE_TYPE_GRAYSCALE
};

void* output = malloc(liblpe_get_compressed_size(&image_info)); // Allocate a buffer that holds the output file

liblpe_status_t result = liblpe_encode(&image_info, input, input_size, output); // Encode the bitmap (input)

if (result) {
  // Handle errors
  error(liblpe_status_to_string(result));
}
```

## Decoding an LPE file
liblpe can decode an LPE file stored in a buffer into either a top-left origin 8-bit grayscale or top-left origin RGB24 bitmap. The following C code is an example:

```c
#include <liblpe.h>

// ...

liblpe_image_info_t image_info;

liblpe_status_t image_info_result = liblpe_decode_image_info(input, input_size, &image_info); // Get the image info struct describing the output bitmap

if (image_info_result) {
  // Handle errors, the file will not decode
  error(liblpe_status_to_string(image_info_result));
}

void* output = malloc(liblpe_get_bitmap_size(&image_info)); // Allocate a buffer that holds the output bitmap

liblpe_status_t result = liblpe_decode(input, input_size, output); // Decode the LPE file (input)

if (result) {
  // Handle errors, the file did not decode
  error(liblpe_status_to_string(result));
}
```
