# lpe-web
Hacky(!!!) JavaScript+WASM script that automatically detects `<img>` tags which attempt to load LPE files, and automatically decodes them client-side with liblpe.

## Installation
With `clang`, `llvm`, `wasm-ld` and `ninja` installed:

```
./configure wasm32
ninja lpe-web
```

The resulting WASM and JS files will be in `bin/lpe-web`.

## Usage
Include `lpe-web.js` as a `defer`ed script on your website, you may also need to adjust the `lpe-web.wasm` location within the script to support your particular setup.
