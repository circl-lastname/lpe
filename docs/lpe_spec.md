# LPE Specification
This is the specification of the LPE (Local Palette Encoding) file format, specifically version 1, it serves as a container for an LPE compressed image.

An LPE file is a standard octet-based file, all binary values contained in it are little endian. The file extenstion used is `.lpe` and its MIME-type may be `image/x-lpe`.

In the case of ambiguity regarding the behavior of an encoder and decoder, the behavior of the reference encoder and decoder ([liblpe](../liblpe)) may be assumed.

## Semantics
An image is organized into channels, which are organized into blocks. Each block represents an 8x8 part of a channel of the image, and contains a 2 item color palette and an 8x8 1-bit depth bitmap. If the image's width and height are not divisible by 8, a block may represent an area smaller than 8x8 at the right and bottom edges of the image, and the pixels it specifies beyond the bounding box of the image shall be ignored when decoding.

When encoding a bitmap, it too shall be split into 8x8 blocks, from which an implementation defined algorithm picks the two most appropriate values from the block from the currently encoded channel (the reference implementation simply picks the highest and lowest values), and uses an implementation defined dithering algorithm to best represent original bitmap using the palette of the current block (the reference implementation uses the simplest error diffusion dithering algorithm, but a more robust algorithm like Floyd-Steinberg may reduce some color innacuracy or perceived sharpness of the image), this information is then stored in the block.

When decoding an LPE file, each block may be simply iterated, and decoded as explained in [Block](#block).

## Header
All LPE files start with the following header:

|Offset|Length|Type of data|Name|Comment|
|-|-|-|-|-|
|`0x00`|`0x03`|8-bit ASCII values|Magic number|The non-null-terminated ASCII string "LPE"|
|`0x03`|`0x01`|Unsigned 8-bit integer|Version and number of channels|The first 4 least significant bits of this integer respresent the version, currently this is always `0x1`. The next 4 bits represent the amount of channels in the image, the value `0x1` represents an 8-bit grayscale image, while a value of `0x3` represents an RGB24 image. A decoder should reject any other value.|
|`0x04`|`0x02`|Unsigned 16-bit integer|Width|The width of the image in pixels, this is also used to calculate the number of blocks horizontally|
|`0x06`|`0x02`|Unsigned 16-bit integer|Height|The height of the image in pixels, this is also used to calculate the number of blocks vertically|

The header is followed by 1 or more [block streams](#block-stream) (unless either the width or height are equal to 0, in which case the file ends after the header), in the case of an 8-bit grayscale image, there is 1 block stream used to represent the single luminance channel, in the case of an RGB24 image, 3 block streams are used to represent R, G, and B channels in that order.

## Block stream
A block stream is an array of the [blocks](#block) of a channel. The amount of items in this array is based on the Width and Height values in the [header](#header), using the equation `ceil(Width/8)` (horizontal blocks) and `ceil(Height/8)` (vertical blocks). The items in this array represent the blocks starting from the top-left, going left-to-right for each column in the row, then top-to-bottom for each row.

## Block
As specified in the [Semantics](#semantics) section, a block represents ideally an 8x8 section of a channel of the image. Its binary format is as follows:

|Offset|Length|Type of data|Name|Comment|
|-|-|-|-|-|
|`0x00`|`0x02`|Unsigned 8-bit integers|Palette|The 2 item palette of the block|
|`0x02`|`0x08`|1-bit depth bitmap|Bitmap|The 8x8 bitmap of the block, each octet encodes a row of pixels inside the block, going top-to-bottom. Going from the least significat bit to the most significant bit, a row of pixels is encoded left-to-right, a value of 0 represents the first value of the palette, and 1 represents the second.|
