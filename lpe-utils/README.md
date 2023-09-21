# lpe-utils
Unix utilities for converting between 8-bit grayscale or RGB24 raw bitmap files and LPE files, or LPE files to BMP files.

## Installation
With `gcc` and `ninja` installed:

```
./configure
ninja lpe-utils
sudo ninja install-lpe-utils
```

## bitmap2lpe
Usage:

```
cat <input> | bitmap2lpe <width> <height> <grayscale/rgb24> > <output.lpe>
```

## lpe2bitmap
Usage:

```
cat <input.lpe> | lpe2bitmap > <output>
```

The utility will print the output bitmap's info like width, height, and encoding.

## lpe2bmp
This converts an LPE file to a typical BMP file, not a raw bitmap.

Usage:

```
cat <input.lpe> | lpe2bmp > <output.bmp>
```
