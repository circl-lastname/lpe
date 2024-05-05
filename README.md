# LPE
Local Palette Encoding (LPE) is a lossy image compression format that provides a compression ratio of 6.4:1 in most cases <sup>1</sup> and acceptable image quality using only simple integer math.

![Tatra](images/tatra.bmp)

A mountain range, as passed through LPE. The [LPE file](images/tatra.lpe) is 92.1 kB in size.

<sup>1</sup> As long as both the height and width of the original bitmap are divisible by 8, this compression ratio is guaranteed. As an example, a 9x9 image will take up the same amount of space as a 16x16 one.

## In this repository
* [LPE Specification](docs/lpe_spec.md) - Specification of the LPE file format
* [LPE gallery](docs/lpe_gallery.md) - Gallery of images passed through LPE.
* [liblpe](liblpe) - The reference implementation of LPE
* [lpe-utils](lpe-utils) - Utilities to convert between bitmaps to LPE files
* [lpe-web](lpe-web) - Hacky JavaScript+WASM script that allows LPE files to be used on websites.
