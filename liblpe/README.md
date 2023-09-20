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
  .type = LIBLPE_TYPE_RGB, // The type of bitmap, this can also be LIBLPE_TYPE_GRAYSCALE
};

void* output = malloc(liblpe_encode_get_output_size(&image_info)); // Allocate a buffer that holds the output file

liblpe_encode(&image_info, input, output); // Encode the bitmap (input)
```

## Decoding an LPE file
liblpe can decode an LPE file stored in a buffer into either a top-left origin 8-bit grayscale or top-left origin RGB24 bitmap. The following C code is an example:

```c
#include <liblpe.h>

// ...

liblpe_image_info_t image_info = liblpe_decode_get_image_info(input); // Get the image info struct describing the output bitmap

void* output = malloc(liblpe_decode_get_output_size(input)); // Allocate a buffer that holds the output bitmap

liblpe_decode(input, output); // Decode the LPE file (input)
```
