//"use strict";

// onsen menu
window.fn = {};

window.fn.open = function() {
	var menu = document.getElementById('menu');
	menu.open();
};

window.fn.load = function(page) {
	var content = document.getElementById('content');
	var menu = document.getElementById('menu');
	content.load(page)
	.then(menu.close.bind(menu));
};

// Variable
var address = "http://192.168.1.1";
//var address = "http://192.168.43.75";
var numPixels = 0;

document.addEventListener('init', function(event) {
	if (event.target.matches('#actions'))
	{
		// Status Variable--------------------------------------------------
		var btnStatus = document.getElementById("btnStatus");
		var textStatus = document.getElementById("textStatus");
		var iconStatus = document.getElementById("iconStatus");
		var popoverStatus = document.getElementById("popoverStatus");

		// Image Variable--------------------------------------------------
		var imgImage = new Image;
		var selectImage = document.getElementById("selectImage");
		var canvasImage =document.getElementById("canvasImage");
		var sliderStart = document.getElementById("sliderStart");
		var textStart = document.getElementById("textStart");
		var sliderStop = document.getElementById("sliderStop");
		var textStop = document.getElementById("textStop");
		var btnLight = document.getElementById("btnLight");
		var btnBurn = document.getElementById("btnBurn");
		var btnStop = document.getElementById("btnStop");
		var btnPlay = document.getElementById("btnPlay");

		// Status Event--------------------------------------------------
		btnStatus.addEventListener('click', function () { popoverStatus.show(btnStatus);}, false);

		// Image Event--------------------------------------------------
		selectImage.addEventListener('change', requestParameterWrite, false);
		imgImage.addEventListener('load', drawImageCanvas, false);
		sliderStart.addEventListener('input', function() { updateTextSlider(sliderStart,textStart, "px");}, false);
		sliderStop.addEventListener('input', function() { updateTextSlider(sliderStop,textStop, "px");}, false);
		sliderStart.addEventListener('input', drawImageCanvas, false);
		sliderStop.addEventListener('input', drawImageCanvas, false);
		sliderStart.addEventListener('change', requestParameterWrite, false);
		sliderStop.addEventListener('change', requestParameterWrite, false);
		btnLight.addEventListener('click', requestLight, false);
		btnBurn.addEventListener('click', requestBurn, false);
		btnStop.addEventListener('click', requestStop, false);
		btnPlay.addEventListener('click', requestPlay, false);

		// Main --------------------------------------------------
		requestFileList();

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
		function updateTextSlider(slider, textSlider, unit)
		{
			textSlider.innerHTML = slider.value + unit;
		}

		//--------------------------------------------------
		function updateStatus(message, color)
		{
			textStatus.innerHTML = message;
			textStatus.style.color = color;
			if (color == 'red')
				iconStatus.setAttribute('icon', 'myStatusRed');

			if (color == 'orange')
				iconStatus.setAttribute('icon', 'myStatusOrange');

			if (color == 'green')
				iconStatus.setAttribute('icon', 'myStatusGreen');
		}

		//--------------------------------------------------
		function setFileList(jsonString, select)
		{
			var json = JSON.parse(jsonString);
			select.options.length = json.fileList.length;

			for (var i = 0; i < json.fileList.length; i++)
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
			// check parameter
			updateTextSlider(sliderStart,textStart, "px");
			updateTextSlider(sliderStop,textStop, "px");
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
	}

	if (event.target.matches('#settings'))
	{
		// Status Variable--------------------------------------------------
		var btnStatus = document.getElementById("btnStatus");
		var textStatus = document.getElementById("textStatus");
		var iconStatus = document.getElementById("iconStatus");
		var popoverStatus = document.getElementById("popoverStatus");

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

		// Action Variable --------------------------------------------------
		var ckInvert = document.getElementById("ckInvert");
		var ckRepeat = document.getElementById("ckRepeat");
		var ckBounce = document.getElementById("ckBounce");
		var ckPause = document.getElementById("ckPause");
		var ckCut = document.getElementById("ckCut");
		var ckEndOff = document.getElementById("ckEndOff");
		var ckEndColor = document.getElementById("ckEndColor");

		// Status Event--------------------------------------------------
		btnStatus.addEventListener('click', function () { popoverStatus.show(btnStatus);}, false);

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

		// Main --------------------------------------------------
		requestParameterRead();

		//--------------------------------------------------
		function updateCheckbox(checkboxFrom, checkboxTo)
		{
			if(checkboxFrom.checked)
			{
				checkboxTo.disabled = true;
			}
			else
			{
				checkboxTo.disabled = false;
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
			if (color == 'red')
				iconStatus.setAttribute('icon', 'myStatusRed');

			if (color == 'orange')
				iconStatus.setAttribute('icon', 'myStatusOrange');

			if (color == 'green')
				iconStatus.setAttribute('icon', 'myStatusGreen');
		}
		
		//--------------------------------------------------
		function setParameter(jsonString)
		{
			var json = JSON.parse(jsonString);
			// set parameters values
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
			// check parameter
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
	}

	if (event.target.matches('#download'))
	{
		// Status Variable--------------------------------------------------
		var btnStatus = document.getElementById("btnStatus");
		var textStatus = document.getElementById("textStatus");
		var iconStatus = document.getElementById("iconStatus");
		var popoverStatus = document.getElementById("popoverStatus");

		// Delete Variable--------------------------------------------------
		var selectDelete = document.getElementById("selectDelete");
		var imgDelete = new Image;
		var canvasDelete =document.getElementById("canvasDelete");
		var btnDelete = document.getElementById("btnDelete");
		var btnDownload = document.getElementById("btnDownload");

		// Status Event--------------------------------------------------
		btnStatus.addEventListener('click', function () { popoverStatus.show(btnStatus);}, false);

		// Delete event--------------------------------------------------
		selectDelete.addEventListener('change', setImgDelete, false);
		imgDelete.addEventListener('load', drawDeleteCanvas, false);
		btnDelete.addEventListener('click', requestFileDelete, false);
		btnDownload.addEventListener('click', requestFileDownload, false);

		// Main --------------------------------------------------
		requestFileList();

		//--------------------------------------------------
		function setImgDelete()
		{
			// test if the file is a bitmap
			var re = /(?:\.([^.]+))?$/;
			var ext = re.exec(selectDelete.value)[1];  
			// it's a bitmap
			if(ext == "bmp")
			{
				imgDelete.src = address + selectDelete.value;
			}
			// it isn't a bitmap
			else
			{
				// context
				var ctx=canvasDelete.getContext("2d");
				// calculate the canvas dimension
				canvasDelete.width = 150;
				canvasDelete.height = 60; 
				// initialize canvas in black
				ctx.fillStyle = "black";
				ctx.fillRect(0, 0, canvasDelete.width, canvasDelete.height);
				// 
				ctx.fillStyle = "red";
				ctx.font = "30px Arial";
				ctx.textAlign = "center";
				ctx.fillText("No Preview", canvasDelete.width/2, canvasDelete.height/2);
			}
		}

		//--------------------------------------------------
		function drawDeleteCanvas()
		{
			// canvas
			var ctx=canvasDelete.getContext("2d");
			// calculate the canvas dimension
			canvasDelete.width = imgDelete.height;
			canvasDelete.height = imgDelete.width; 
			// save context
			ctx.save();
			// translate and rotate
			ctx.translate(canvasDelete.width/2,canvasDelete.height/2);
			ctx.rotate(-90*Math.PI/180);
			// draw imgDelete in canvasDelete
			ctx.drawImage(imgDelete, -canvasDelete.height/2, -canvasDelete.width/2);
			//restore context
			ctx.restore();
		}

		//--------------------------------------------------
		function updateStatus(message, color)
		{
			textStatus.innerHTML = message;
			textStatus.style.color = color;
			if (color == 'red')
				iconStatus.setAttribute('icon', 'myStatusRed');

			if (color == 'orange')
				iconStatus.setAttribute('icon', 'myStatusOrange');

			if (color == 'green')
				iconStatus.setAttribute('icon', 'myStatusGreen');
		}

		//--------------------------------------------------
		function setFileList(jsonString, select)
		{
			var json = JSON.parse(jsonString);
			select.options.length = json.fileList.length;

			for (var i = 0; i < json.fileList.length; i++)
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
					setFileList(this.responseText,selectDelete);
					setImgDelete();
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
		function requestFileDownload()
		{
			var a  = document.createElement('a');
			a.href = address + selectDelete.value;
			a.download = selectDelete.value.substring(1);
			a.click();
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
	}

	if (event.target.matches('#upload'))
	{
		// Status Variable--------------------------------------------------
		var btnStatus = document.getElementById("btnStatus");
		var textStatus = document.getElementById("textStatus");
		var iconStatus = document.getElementById("iconStatus");
		var popoverStatus = document.getElementById("popoverStatus");

		// Convert Variable--------------------------------------------------
		var imgConvert = new Image;
		var canvasConvert = document.getElementById("canvasConvert");
		var selectConvert = document.getElementById("selectConvert");
		var btnUploadOriginal = document.getElementById("btnUploadOriginal");
		var btnUploadConvert = document.getElementById("btnUploadConvert");
		var btnDownloadConvert = document.getElementById("btnDownloadConvert");

		// Status Event--------------------------------------------------
		btnStatus.addEventListener('click', function () { popoverStatus.show(btnStatus);}, false);

		// Convert event--------------------------------------------------
		selectConvert.addEventListener('change', setImgConvert, false);
		imgConvert.addEventListener('load', drawConvertCanvas, false);
		btnUploadOriginal.addEventListener('click', uploadOriginal, false);
		btnDownloadConvert.addEventListener('click', downloadConvert, false);
		btnUploadConvert.addEventListener('click', uploadConvert, false);

		// Main --------------------------------------------------
		requestParameterRead();
		setImgConvert();

		//--------------------------------------------------
		function setImgConvert()
		{  
			// no selection
			if (selectConvert.files.length == 0)
			{
				// print the error
				errorConvertCanvas("No File");
				// button
				btnUploadOriginal.setAttribute('disabled', '');
				btnUploadConvert.setAttribute('disabled', '');
				btnDownloadConvert.setAttribute('disabled', '');
				return;
			}
			// test the selection
			var file = selectConvert.files[0];
			var imageType = /^image\//;
			// selection is not an image
			if (!imageType.test(file.type))
			{
				// print the error
				errorConvertCanvas("No Convert");
				// button
				btnUploadOriginal.removeAttribute('disabled', '');
				btnUploadConvert.setAttribute('disabled', '');
				btnDownloadConvert.setAttribute('disabled', '');
			}
			// selection is an image
			else
			{
				// load the image
				imgConvert.file = file;
				var reader = new FileReader();
				reader.onload = (function(aImg) { return function(e) { aImg.src = e.target.result; }; })(imgConvert); 
				reader.readAsDataURL(file);
				// button
				btnUploadOriginal.removeAttribute('disabled', '');
				btnUploadConvert.removeAttribute('disabled', '');
				btnDownloadConvert.removeAttribute('disabled', '');
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
		function errorConvertCanvas(error)
		{
		// context
		var ctx=canvasConvert.getContext("2d");
		// calculate the canvas dimension
		canvasConvert.width = 150;
		canvasConvert.height = 60; 
		// initialize canvas in black
		ctx.fillStyle = "black";
		ctx.fillRect(0, 0, canvasConvert.width, canvasConvert.height);
		// 
		ctx.fillStyle = "red";
		ctx.font = "30px Arial";
		ctx.textAlign = "center";
		ctx.fillText(error, canvasConvert.width/2, canvasConvert.height/2);
	}

		//--------------------------------------------------
		function updateStatus(message, color)
		{
			textStatus.innerHTML = message;
			textStatus.style.color = color;
			if (color == 'red')
				iconStatus.setAttribute('icon', 'myStatusRed');

			if (color == 'orange')
				iconStatus.setAttribute('icon', 'myStatusOrange');

			if (color == 'green')
				iconStatus.setAttribute('icon', 'myStatusGreen');
		}

		//--------------------------------------------------
		function setParameter(jsonString)
		{
			var json = JSON.parse(jsonString);
			// set parameters values
			numPixels = json["numPixels"];
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
		function trimFileName(fileName, newExt)
		{
			var trimName;
			// test
			var extRegex = /(?:\.([^.]+))?$/;
			// retrieve current extension
			var currentExt = extRegex.exec(fileName)[1];
			// retrieve base name
			var baseName = fileName.substring(0, fileName.length-(currentExt.length+1));
			// spiffs support maxi 31 characters (extension include) so we trim the baseName to 20 characters for security
			if (baseName.length > 20)
			{
				baseName=baseName.substring(0, 20);
			}
            // 
            if (newExt == "")
            {
            	trimName = baseName+"."+currentExt;
            }
			//
			else
			{
				trimName =  baseName+"."+newExt;
			}
			return trimName;
		}

		//--------------------------------------------------
		function downloadConvert()
		{
			var bitmap    = CanvasToBMP.toDataURL(canvasConvert);
			var a  = document.createElement('a');
			a.href = bitmap;
			a.download = trimFileName(selectConvert.files[0].name, "bmp");
			a.click();
		}

		//--------------------------------------------------
		function uploadConvert()
		{
			var form = new FormData();
			form.append('file', CanvasToBMP.toBlob(canvasConvert),trimFileName(selectConvert.files[0].name, "bmp"));
			requestFileUpload(form);
		}

		//--------------------------------------------------
		function uploadOriginal()
		{
			var form = new FormData();
			form.append('file', selectConvert.files[0], trimFileName(selectConvert.files[0].name, ""));
			requestFileUpload(form);
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
	}

	if (event.target.matches('#system'))
	{
		// Status Variable--------------------------------------------------
		var btnStatus = document.getElementById("btnStatus");
		var textStatus = document.getElementById("textStatus");
		var iconStatus = document.getElementById("iconStatus");
		var popoverStatus = document.getElementById("popoverStatus");

		// System Variable--------------------------------------------------
		var selectAddress = document.getElementById("selectAddress");
		var btnAddress = document.getElementById("btnAddress");
		var theme = document.getElementById("theme");
		var btnThemeLight = document.getElementById("btnThemeLight");
		var btnThemeDark = document.getElementById("btnThemeDark");

		// Status Event--------------------------------------------------
		btnStatus.addEventListener('click', function () { popoverStatus.show(btnStatus);}, false);

		// System Event--------------------------------------------------
		btnAddress.addEventListener('click', function () { address = selectAddress.value;}, false);
		btnThemeLight.addEventListener('click',function () {theme.setAttribute('href', 'css/onsen-css-components.min.css'); }, false);
		btnThemeDark.addEventListener('click',function () {theme.setAttribute('href', 'css/dark-onsen-css-components.min.css'); }, false);

		//Main--------------------------------------------------
		selectAddress.value = address;

	}

}, false);
