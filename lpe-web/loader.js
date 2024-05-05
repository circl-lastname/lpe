const importObject = {
  env: {
    print_int: (arg) => {
      console.log(arg);
    },
    get_lpe_size: () => {
      return lpeSize;
    },
    get_lpe_byte: () => {
      return lpeView[lpeIndex++];
    }
  }
};

let lpeModule;
let lpeView;
let lpeIndex;
let lpeSize;

async function main() {
  if (!lpeModule) {
    lpeModule = await WebAssembly.instantiateStreaming(fetch("lpe-web.wasm"), importObject);
  }
  
  let imgElements = document.getElementsByTagName("img");
  
  let postFuncs = [];
  
  for (let i = 0; i < imgElements.length; i++) {
    if (imgElements[i].src.endsWith(".lpe")) {
      let lpeFile = await fetch(imgElements[i].src);
      
      lpeView = new Uint8Array(await lpeFile.arrayBuffer());
      lpeSize = lpeView.length;
      
      lpeIndex = 0;
      
      if (lpeModule.instance.exports.decode_lpe() != 1) {
        console.log("Failed to decode element", imgElements[i]);
        continue;
      }
      
      let width = lpeModule.instance.exports.get_bitmap_width();
      let height = lpeModule.instance.exports.get_bitmap_height();
      
      let bitmap = new ImageData(width, height);
      let bitmapSize = lpeModule.instance.exports.get_bitmap_size();
      
      let k = 0;
      
      for (let j = 0; j < bitmapSize;) {
        bitmap.data[k] = lpeModule.instance.exports.get_bitmap_byte(); // R
        j++; k++;
        bitmap.data[k] = lpeModule.instance.exports.get_bitmap_byte(); // G
        j++; k++;
        bitmap.data[k] = lpeModule.instance.exports.get_bitmap_byte(); // B
        j++; k++;
        bitmap.data[k] = 255; // A
        k++;
      }
      
      let canvas = document.createElement("canvas");
      canvas.width = width;
      canvas.height = height;
      
      let ctx = canvas.getContext("2d");
      ctx.putImageData(bitmap, 0, 0);
      
      let thisImgElement = imgElements[i];
      
      postFuncs.push(() => {
        thisImgElement.replaceWith(canvas);
      });
    }
  }
  
  for (let func in postFuncs) {
    postFuncs[func]();
  }
}

main();
