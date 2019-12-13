//--------------------------------------------------
var CanvasToBMP = {

	//Convert a canvas element to ArrayBuffer containing a BMP file
	toArrayBuffer: function(canvas) {
		var w = canvas.width,
		h = canvas.height,
		w3 = w * 3,
		idata = canvas.getContext("2d").getImageData(0, 0, w, h),
				data32 = new Uint32Array(idata.data.buffer),				// 32-bit representation of canvas
				headerSize = 14,
				DIBHeaderSize = 40,
				//stride = Math.floor((Math.floor((w*24+7)/8)+3)/4)*4,		// row length incl. padding
				stride = Math.floor((24 * w + 31) / 32 ) * 4, 	         	// row length incl. padding
				pixelArraySize = stride * h,								// total bitmap size
				fileLength = headerSize + DIBHeaderSize + pixelArraySize,	// header size is known + bitmap
				file = new ArrayBuffer(fileLength),							// raw byte buffer (returned)
				view = new DataView(file),									// handle endian, reg. width etc.
				pos = 0,
				s = 0;

		// write file header
		setU16(0x4d42);						// BM
		setU32(fileLength);					// total length
		pos += 4;							// skip unused fields
		setU32(headerSize + DIBHeaderSize);	// offset to pixels

		// DIB header
		setU32(DIBHeaderSize);				// header size
		setU32(w);
		setU32(h >>> 0);					// negative = bottom-to-top
		setU16(1);							// 1 plane
		setU16(24);							// 24-bits (RGB)
		setU32(0);							// no compression (BI_BITFIELDS, 3)
		setU32(pixelArraySize);				// bitmap size incl. padding (stride x height)
		setU32(0x2e23);						// pixels/meter h (~72 DPI x 39.3701 inch/m)
		setU32(0x2e23);						// pixels/meter v
		pos += 8;							// skip color/important colors

		// bitmap data, change order of ABGR to BGR
		for (let y = 0; y < h; y++) {
			const p = headerSize + DIBHeaderSize + ((h - y - 1) * stride); // offset + stride x height
			for (let x = 0; x < w3; x += 3) {
				const v = data32[s++];			// get ABGR

				const r = v % 256;
				const g = (v >>> 8) % 256;
				const b = (v >>> 16) % 256;

				view.setUint8(p + x, b);		// red channel
				view.setUint8(p + x + 1, g);	// green channel
				view.setUint8(p + x + 2, r);	// blue channel
			}
		}

		return file;

		// helper method to move current buffer position
		function setU16(data) {view.setUint16(pos, data, true); pos += 2}
		function setU32(data) {view.setUint32(pos, data, true); pos += 4}
	},

	//Converts a canvas to BMP file, returns a Blob representing the file. This can be used with URL.createObjectURL().

	toBlob: function(canvas) {
		return new Blob([this.toArrayBuffer(canvas)], {
			type: "image/bmp"
		});
	},

	//Converts the canvas to a data-URI representing a BMP file.
	toDataURL: function(canvas) {
		var buffer = new Uint8Array(this.toArrayBuffer(canvas)),
		bs = "", i = 0, l = buffer.length;
		while (i < l) bs += String.fromCharCode(buffer[i++]);
		return "data:image/bmp;base64," + btoa(bs);
	}
};
