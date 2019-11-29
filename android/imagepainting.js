// IP
var address = "http://192.168.43.252";
var numPixels = 0;

// Status Variable--------------------------------------------------
var textStatus = document.getElementById("textStatus");

// Delete Variable--------------------------------------------------
var selectDelete = document.getElementById("selectDelete");
var btnDelete = document.getElementById("btnDelete");
var btnDownload = document.getElementById("btnDownload");

// Upload Variable--------------------------------------------------
var selectUpload = document.getElementById("selectUpload");
var btnUploadOriginal = document.getElementById("btnUploadOriginal");

// Convert Variable--------------------------------------------------
var imgConvert = new Image;
var canvasConvert = document.getElementById("canvasConvert");
var selectConvert = document.getElementById("selectConvert");
var btnDownloadConvert = document.getElementById("btnDownloadConvert");
var btnUploadConvert = document.getElementById("btnUploadConvert");

// Settings Variable--------------------------------------------------
var sliderDelay = document.getElementById("sliderDelay");
var textDelay = document.getElementById("textDelay");
var sliderBrightness = document.getElementById("sliderBrightness");
var textBrightness = document.getElementById("textBrightness");
var sliderRepeat = document.getElementById("sliderRepeat");
var textRepeat = document.getElementById("textRepeat");
var sliderPause = document.getElementById("sliderPause");
var textPause = document.getElementById("textPause");
var pickerColor = document.getElementById("pickerColor");

// Image Variable--------------------------------------------------
var imgImage = new Image;
var canvasImage =document.getElementById("canvasImage");
var sliderStart = document.getElementById("sliderStart");
var textStart = document.getElementById("textStart");
var sliderStop = document.getElementById("sliderStop");
var textStop = document.getElementById("textStop");

// Action Variable --------------------------------------------------
var ckInvert = document.getElementById("ckInvert");
var ckRepeat = document.getElementById("ckRepeat");
var ckBounce = document.getElementById("ckBounce");
var ckPause = document.getElementById("ckPause");
var ckCut = document.getElementById("ckCut");
var ckEndOff = document.getElementById("ckEndOff");
var ckEndColor = document.getElementById("ckEndColor");
var selectImage = document.getElementById("selectImage");
var btnLight = document.getElementById("btnLight");
var btnBurn = document.getElementById("btnBurn");
var btnStop = document.getElementById("btnStop");
var btnPlay = document.getElementById("btnPlay");

// Delete event--------------------------------------------------
btnDelete.addEventListener('click', requestFileDelete, false);
btnDownload.addEventListener('click', downloadFile, false);

// Upload event--------------------------------------------------
btnUploadOriginal.addEventListener('click', uploadOriginal, false);

// Convert event--------------------------------------------------
selectConvert.addEventListener('change', drawImage, false);
imgConvert.addEventListener('load', drawConvertCanvas, false);
btnDownloadConvert.addEventListener('click', downloadConvert, false);
btnUploadConvert.addEventListener('click', uploadConvert, false);

// Settings Event--------------------------------------------------
sliderDelay.addEventListener('input', function() { updateTextSlider(sliderDelay,textDelay, "ms");}, false);
sliderBrightness.addEventListener('input', function() {updateTextSlider(sliderBrightness,textBrightness, "%");}, false);
sliderRepeat.addEventListener('input', function() {updateTextSlider(sliderRepeat,textRepeat, "x");}, false);
sliderPause.addEventListener('input', function() {updateTextSlider(sliderPause,textPause, "px");}, false);
sliderDelay.addEventListener('change', requestParameterWrite, false);
sliderBrightness.addEventListener('change', requestParameterWrite, false);
sliderRepeat.addEventListener('change', requestParameterWrite, false);
sliderPause.addEventListener('change', requestParameterWrite, false);
pickerColor.addEventListener('change', requestParameterWrite, false);

// Image Event--------------------------------------------------
selectImage.addEventListener('change', requestParameterWrite, false);
imgImage.addEventListener('load', drawImageCanvas, false);
sliderStart.addEventListener('input', function() { updateTextSlider(sliderStart,textStart, "px");}, false);
sliderStop.addEventListener('input', function() { updateTextSlider(sliderStop,textStop, "px");}, false);
sliderStart.addEventListener('input', drawImageCanvas, false);
sliderStop.addEventListener('input', drawImageCanvas, false);
sliderStart.addEventListener('change', requestParameterWrite, false);
sliderStop.addEventListener('change', requestParameterWrite, false);

// Action event--------------------------------------------------
ckRepeat.addEventListener('click', function() {updateCheckbox(ckRepeat,ckBounce);}, false);
ckBounce.addEventListener('click', function() {updateCheckbox(ckBounce,ckRepeat);}, false);
ckPause.addEventListener('click', function() {updateCheckbox(ckPause,ckCut);}, false);
ckCut.addEventListener('click', function() {updateCheckbox(ckCut,ckPause);}, false);
ckEndColor.addEventListener('click', function() {updateCheckbox(ckEndColor,ckEndOff);}, false);
ckEndOff.addEventListener('click', function() {updateCheckbox(ckEndOff,ckEndColor);}, false);
ckInvert.addEventListener('click', requestParameterWrite, false);
ckRepeat.addEventListener('click', requestParameterWrite, false);
ckBounce.addEventListener('click', requestParameterWrite, false);
ckPause.addEventListener('click', requestParameterWrite, false);
ckCut.addEventListener('click', requestParameterWrite, false);
ckEndColor.addEventListener('click', requestParameterWrite, false);
ckEndOff.addEventListener('click', requestParameterWrite, false);
btnLight.addEventListener('click', requestLight, false);
btnBurn.addEventListener('click', requestBurn, false);
btnStop.addEventListener('click', requestStop, false);
btnPlay.addEventListener('click', requestPlay, false);


// Main --------------------------------------------------
requestFileList();

//--------------------------------------------------
function drawImage()
{   var file = selectConvert.files[0];
	var imageType = /^image\//;
	if (selectConvert.files.length == 0 || !imageType.test(file.type))
	{
		updateStatus("CONVERT ERROR : SELECT AN IMAGE", "red");
		selectConvert.value = "";
		imgConvert.src = "#";
		canvasConvert.height = 0;
	}
	else
	{
		updateStatus("CONVERT SUCCESS", "green");
		imgConvert.file = file;
		var reader = new FileReader();
		reader.onload = (function(aImg) { return function(e) { aImg.src = e.target.result; }; })(imgConvert); 
		reader.readAsDataURL(file);
	}
}

//--------------------------------------------------
function drawConvertCanvas()
{
	// canvas
	var ctx = canvasConvert.getContext("2d");
	// calculate the canvas dimension
	canvasConvert.width = numPixels;
	canvasConvert.height = imgConvert.width/imgConvert.height*numPixels; 
	// initialize canvas in black
	ctx.fillRect(0, 0, canvasConvert.width, canvasConvert.height);
	// save context
	ctx.save();
	// translate and rotate
	ctx.translate(canvasConvert.width/2,canvasConvert.height/2);
	ctx.rotate(90*Math.PI/180);
	// draw imgConvert strech to fit canvasConvert
	ctx.drawImage(imgConvert,-canvasConvert.height/2,-canvasConvert.width/2,canvasConvert.height,canvasConvert.width);
	//restore context
	ctx.restore();
}

//--------------------------------------------------
function drawImageCanvas()
{
	// canvas
	var ctx=canvasImage.getContext("2d");
	// calculate the canvas dimension
	canvasImage.width = imgImage.height;
	canvasImage.height = imgImage.width; 
	// initialize canvas in black
	ctx.fillRect(0, 0, canvasImage.width, canvasImage.height);
	// save context
	ctx.save();
	// translate and rotate
	ctx.translate(canvasImage.width/2,canvasImage.height/2);
	ctx.rotate(-90*Math.PI/180);
	// draw imgImage in canvasImage
	ctx.drawImage(imgImage, -canvasImage.height/2, -canvasImage.width/2);
	//restore context
	ctx.restore();
	// curtain color
	ctx.fillStyle = "red";
	// draw curtain
	ctx.fillRect(0, 0, sliderStart.value, canvasImage.height);
	ctx.fillRect(Number(sliderStop.value)+1, 0, canvasImage.width-sliderStop.value, canvasImage.height);
}

//--------------------------------------------------
function updateCheckbox(checkboxFrom, checkboxTo)
{
	if(checkboxFrom.checked)
	{
		checkboxTo.disabled = true;
		checkboxTo.parentNode.style.color = "grey";
	}
	else
	{
		checkboxTo.disabled = false;
		checkboxTo.parentNode.style.color = "white";
	}
}

//--------------------------------------------------
function updateTextSlider(slider, textSlider, unit)
{
	textSlider.innerHTML = slider.value + unit;
}

//--------------------------------------------------
function updateStatus(message, color)
{
	textStatus.innerHTML = message;
	textStatus.style.color = color;
}

//--------------------------------------------------
function setFileList(jsonString, select)
{
	var json = JSON.parse(jsonString);
	select.options.length = json.fileList.length;

	for (i = 0; i < json.fileList.length; i++)
	{
		select.options[i].value = json.fileList[i]; 
		select.options[i].text = json.fileList[i]; 
	}
}

//--------------------------------------------------
function requestFileList()
{
	var xhr = new XMLHttpRequest();
	xhr.onload = function()
	{
		if (this.status == 200)
		{
			setFileList(this.responseText,selectImage);
			setFileList(this.responseText,selectDelete);
			requestParameterRead();
		}
	};

	xhr.onerror = function()
	{
		updateStatus("LIST ERROR : CONNECTION LOST", "red");
	};
	// send the request
	xhr.overrideMimeType("application/json");
	xhr.open("GET", address + "/list", true);
	xhr.send(null);
}
//--------------------------------------------------
function requestPlay()
{
	requestAction("/play");
}

//--------------------------------------------------
function requestStop()
{
	requestAction("/stop");
}

//--------------------------------------------------
function requestBurn()
{
	requestAction("/burn");
}

//--------------------------------------------------
function requestLight()
{
	requestAction("/light");
}

//--------------------------------------------------
function requestAction(action)
{
	var xhr = new XMLHttpRequest();
	xhr.onload = function()
	{
		if (this.status == 200)
		{
			updateStatus(this.responseText, "green");
		}
		else
		{
			updateStatus(this.responseText, "red");
		}
	};

	xhr.onerror = function()
	{
		updateStatus("ACTION ERROR : CONNECTION LOST", "red");
	};

	xhr.open("GET", address + action, true);
	xhr.send(null);
}

//--------------------------------------------------
function setParameter(jsonString)
{
	var json = JSON.parse(jsonString);
	// set parameters values
	sliderStart.setAttribute("min", json["indexMin"]);
	sliderStart.setAttribute("max", json["indexMax"]);
	sliderStart.value = json["indexStart"];
	sliderStop.setAttribute("min", json["indexMin"]);
	sliderStop.setAttribute("max", json["indexMax"]);
	sliderStop.value = json["indexStop"];
	selectImage.value = json["bmpPath"];
	imgImage.src= address + json["bmpPath"];
	sliderDelay.value = json["delay"];
	sliderBrightness.value = json["brightness"];
	sliderRepeat.value = json["repeat"];
	sliderPause.value = json["pause"];
	pickerColor.value = json["color"];
	ckInvert.checked  = json["isinvert"];
	ckRepeat.checked  = json["isrepeat"];
	ckBounce.checked  = json["isbounce"];
	ckPause.checked  = json["ispause"];
	ckCut.checked  = json["iscut"];
	ckEndOff.checked  = json["isendoff"];
	ckEndColor.checked  = json["isendcolor"];
	numPixels = json["numPixels"];
	// check parameter
	updateTextSlider(sliderStart,textStart, "px");
	updateTextSlider(sliderStop,textStop, "px");
	updateTextSlider(sliderDelay,textDelay, "ms");
	updateTextSlider(sliderBrightness,textBrightness, "%");
	updateTextSlider(sliderRepeat,textRepeat, "x");
	updateTextSlider(sliderPause,textPause, "px");
	updateCheckbox(ckRepeat,ckBounce);
	updateCheckbox(ckBounce,ckRepeat);
	updateCheckbox(ckPause,ckCut);
	updateCheckbox(ckCut,ckPause);
	updateCheckbox(ckEndOff,ckEndColor);
	updateCheckbox(ckEndColor,ckEndOff);
}

//--------------------------------------------------
function requestParameterRead()
{
	var xhr = new XMLHttpRequest();
	xhr.onload = function()
	{
		if (this.status == 200)
		{
			setParameter(this.responseText);
		}
	};

	xhr.onerror = function()
	{
		updateStatus("READ ERROR : CONNECTION LOST", "red");
	};

	xhr.overrideMimeType("application/json");
	xhr.open("GET", address+"/parameterRead", true);
	xhr.send(null);
}

//--------------------------------------------------
function getParameter()
{
	var json = new Object();
	// get parameters
	json.delay = sliderDelay.value;
	json.brightness = sliderBrightness.value;
	json.repeat = sliderRepeat.value;
	json.pause = sliderPause.value;
	json.color = pickerColor.value;
	json.isrepeat = ckRepeat.checked;
	json.isbounce = ckBounce.checked;
	json.ispause = ckPause.checked;
	json.iscut = ckCut.checked;
	json.isinvert = ckInvert.checked;
	json.isendoff = ckEndOff.checked;
	json.isendcolor = ckEndColor.checked;
	json.indexStart = sliderStart.value;
	json.indexStop = sliderStop.value;
	json.bmpPath = selectImage.value;
	// convert json to string
	return JSON.stringify(json);
}

//--------------------------------------------------
function requestParameterWrite()
{
	var xhr = new XMLHttpRequest();
	xhr.onload = function()
	{
		if (this.status == 200)
		{
			updateStatus(this.responseText, "green");
		}
		else
		{
			updateStatus(this.responseText, "red");
		}
		requestParameterRead();
	};

	xhr.onerror = function()
	{
		updateStatus("WRITE ERROR : CONNECTION LOST", "red");
	};

	// send the request
	xhr.open("POST", address+"/parameterWrite", true);
	xhr.setRequestHeader('Content-type', 'application/json');
	xhr.send(getParameter());
}

//--------------------------------------------------
function downloadFile()
{
	var a  = document.createElement('a');
	a.href = selectDelete.value;
	a.download = selectDelete.value.substring(1);
	a.click()
}

//--------------------------------------------------
function requestFileDelete()
{
	var xhr = new XMLHttpRequest();
	xhr.onload = function()
	{
		if (this.status == 200)
		{
			updateStatus(this.responseText, "green");
			requestFileList();
		}
		else
		{
			updateStatus(this.responseText, "red");
		}
	};
	
	xhr.onerror = function()
	{
		updateStatus("DELETE ERROR : CONNECTION LOST", "red");
	};
	
	xhr.open("DELETE", address+"/delete", true);
	xhr.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
	xhr.send(selectDelete.value);
}

//--------------------------------------------------
function downloadConvert()
{
	if (selectConvert.files.length == 0)
	{
		updateStatus("CONVERT ERROR : SELECT AN IMAGE", "red");
	}
	else
	{
		updateStatus("CONVERT SUCCESS", "green");
		var bitmap    = CanvasToBMP.toDataURL(canvasConvert);
		var a  = document.createElement('a');
		a.href = bitmap;
		a.download = trimFileName(selectConvert.files[0].name)+ ".bmp";
		a.click()
	}
}

//--------------------------------------------------
function uploadConvert()
{
	if (selectConvert.files.length == 0)
	{
		updateStatus("CONVERT ERROR : SELECT AN IMAGE", "red");
	}
	else
	{
		var form = new FormData();
		form.append('file',  CanvasToBMP.toBlob(canvasConvert),trimFileName(selectConvert.files[0].name)+ ".bmp");
		requestFileUpload(form);
	}
}

//--------------------------------------------------
function trimFileName(filename)
{
	// remove extension
	var trimfilename = filename.split('.').slice(0, -1).join('.');
	// spiffs support maxi 31 characters (extension include) so we trim the name to 27 characters
	if (trimfilename.length > 27)
	{
		trimfilename=trimfilename.substring(0, 26);
	}
	return trimfilename;
}

//--------------------------------------------------
function uploadOriginal()
{
	if (selectUpload.files.length == 0)
	{
		updateStatus("UPLOAD ERROR : SELECT A FILE", "red");
	}
	else
	{
		var form = new FormData();
		form.append('file', selectUpload.files[0]);
		requestFileUpload(form);
	}
}

//--------------------------------------------------
function requestFileUpload(form)
{
	var xhr = new XMLHttpRequest();
	xhr.onload = function()
	{
		if (this.status == 200)
		{
			updateStatus(this.responseText, "green");
			requestFileList();
		}
		else
		{
			updateStatus("UPLOAD ERROR : UPLOAD FAILED", "red");
		}
	};
	
	xhr.upload.onprogress = function(evt)
	{
		var percentComplete = Math.floor(evt.loaded / evt.total * 100);
		updateStatus("UPLOAD PROGRESS :"+percentComplete+"%", "orange");
	};
	
	xhr.onerror = function()
	{
		updateStatus("UPLOAD ERROR : CONNECTION LOST", "red");
	};
	
	xhr.open("POST", address+"/upload", true);
	xhr.send(form);
}

//--------------------------------------------------
var CanvasToBMP = {

//Convert a canvas element to ArrayBuffer containing a BMP file
toArrayBuffer: function(canvas) {
	var w = canvas.width,
	h = canvas.height,
	w3 = w * 3,
	idata = canvas.getContext("2d").getImageData(0, 0, w, h),
			data32 = new Uint32Array(idata.data.buffer), // 32-bit representation of canvas
			headerSize = 14,
			DIBHeaderSize = 40,
			stride = Math.floor((24 * w + 23) / 24) * 3, // row length incl. padding
			pixelArraySize = stride * h,                 // total bitmap size
			fileLength = headerSize + DIBHeaderSize + pixelArraySize,           // header size is known + bitmap

			file = new ArrayBuffer(fileLength),          // raw byte buffer (returned)
			view = new DataView(file),                   // handle endian, reg. width etc.
			pos = 0, s = 0;

		// write file header
		setU16(0x4d42);          // BM
		setU32(fileLength);      // total length
		pos += 4;                // skip unused fields
		setU32(headerSize + DIBHeaderSize);            // offset to pixels

		// DIB header
		setU32(DIBHeaderSize);             // header size
		setU32(w);
		setU32(h >>> 0);        // negative = bottom-to-top
		setU16(1);               // 1 plane
		setU16(24);              // 24-bits (RGB)
		setU32(0);               // no compression (BI_BITFIELDS, 3)
		setU32(pixelArraySize);  // bitmap size incl. padding (stride x height)
		setU32(0x2e23);            // pixels/meter h (~72 DPI x 39.3701 inch/m)
		setU32(0x2e23);            // pixels/meter v
		pos += 8;                // skip color/important colors

		// bitmap data, change order of ABGR to BGR
		for (let y = 0; y < h; y++) {
			const p = headerSize + DIBHeaderSize + ((h - y - 1) * stride); // offset + stride x height
			for (let x = 0; x < w3; x += 3) {
				const v = data32[s++];                     // get ABGR

				const r = v % 256;
				const g = (v >>> 8) % 256;
				const b = (v >>> 16) % 256;

				view.setUint8(p + x, b);                      // red channel
				view.setUint8(p + x + 1, g);                  // green channel
				view.setUint8(p + x + 2, r);                  // blue channel
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