# ImagePainting

Yet another project on LightPainting .... BUT WAIT !!!

## Introduction

Existing project on the topic like Adafruit [NeoPixel Painter](https://learn.adafruit.com/neopixel-painter/overview) or [Light Painting with Raspberry Pi](https://learn.adafruit.com/light-painting-with-raspberry-pi/overview) are great and have been a great source of inspiration/information, but i find those too complicated and unpractical on the field. You need too much hardware and you can't be far from your computer to load other image or tweak parameters.

My project is base on hardware and use simplicity. All you need is an USB battery bank, a LED Strip, an ESP8266 board and a webbrowser (computer of course but mainly smartphone).

After a simple hardware assembly and flashing my code, all actions (image upload/delete, start, pause, stop, light) and parameters (delay, brightness, repeat, bounce, invert) are send through your webbrowser. So load your smartphone with your picture and you are ready to shoot.

## Hardware Prerequisites

![hardware](https://user-images.githubusercontent.com/2498942/68552594-e7a07400-0418-11ea-9d8a-11a54ea146d7.jpg)

* **USB Battery bank :** I use sony CP-V3A for 60 LEDS. Take care of the output amps and use regular USB.

* **ESP8266 Board :** I use Wemos D1 mini clone, but i should work on other ESP8266 base board like NodeMCU.

* **LED Strip :** I use APA102. It could work on others LEDS like WS2812, but APA102 are better and faster.

* **Push Button (optional) :** Any Push button should be good.

## Hardware Assembly

The following hardware assembly are 100% working for me, but could be improve by adding a capacitor to filter the power and a level schifter to convert the 3.3v logic to 5v for the LEDs. The best option will be this shield [Hex Wemos D1 Mini Wi-Fi LED Controller](https://www.evilgeniuslabs.org/hex-wemos-d1-mini-wifi-led-controller) from Evil Genius. All the drawing i give are for the Wemos D1 mini and clone, if you use an other ESP8266 board read carefully her spec.

### LED Strip connection

I have test with success Dotstars (aka APA102) and Neopixels (aka WS2812B). I have selected the fastest METHOD for this two type of LED, but you can find more options at the [NeoPixelBus](https://github.com/Makuna/NeoPixelBus/wiki/NeoPixelBus-object#neo-methods) dedicated page. If you have a problem with color try to select the rigth FEATURE : Brg, Grb, Rgb... are the order of the Red, Green and Blue componement inside each LED which can vary. So FEATURE and METHOD are define by this two macro at the beginning of the sketch :

```
#define FEATURE DotStarBgrFeature // Neopixels : NeoGrbFeature / Dotstars : DotStarBgrFeature
#define METHOD DotStarSpiMethod // Neopixels :Neo800KbpsMethod / Dotstars : DotStarSpiMethod
```

![help6](https://user-images.githubusercontent.com/2498942/69486316-7c0fcb00-0e4a-11ea-962f-718bfbf4173d.png)

* **Neopixels :** FEATURE : Neo###Feature and METHOD : Neo800KbpsMethod. You must use RDX0/GPIO3 PIN (label RX on Wemos D1 mini) as specify [here](https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods#neoesp8266dma800kbpsmethod).


![help5](https://user-images.githubusercontent.com/2498942/69486235-0c99db80-0e4a-11ea-9101-3f5566b0bd12.png)

* **Dotstars :** FEATURE : DotStar###Feature and METHOD : DotStarSpiMethod. You must use the hardware SPI pin from your board (DATA = MOSI and CLOCK = SCLK). For the Wemos D1 mini MOSI is D7 and SCLK is D5.

### Push button connection

```
#define BUTTON      // Decomment this to use BUTTON
```

The 2 push buttons are optionals. ImagePainting always work from the webpage and you still need your smartphone to upload Bitmap or tweak parameters. However, it's more convenient to launch your animation with a button than clicking on a touch screen.

By defaut, ImagePainting handle the 2 push buttons (BTNA and BTNB) like this :

* **BTNA :** "short click" --> Play/Pause and "long click" --> Burn

* **BTNB :** "short click" --> Stop and "long click" --> Light

This section of the code define the push button :

```
// BUTTON --------------
long DEBOUNCETIME = 50; //Debouncing Time in Milliseconds
long HOLDTIME = 500; // Hold Time in Milliseconds
const int BTNA_PIN = D3; //PIN for the button A
const int BTNB_PIN = D4; //PIN for the button B
long BTNATIMER = 0;
long BTNBTIMER = 0;
boolean ISBTNA = false;
boolean ISBTNB = false;
boolean ISBTNAHOLD = false;
boolean ISBTNBHOLD = false;
// end BUTTON-----------
```

* **Change the PIN :** I use D3 and D4 on purpose for my Wemos D1. This is internal Pull-Up Pin, so check the specification of your board before changing those PIN.

* **HOLDTIME :** HOLDTIME can be tweak to have a shorter/longer "long click".

* **DEBOUNCETIME :** DEBOUNCETIME can be tweak to have a good button response without false detection.

### USB battery bank connection

![help4](https://user-images.githubusercontent.com/2498942/69486213-b036bc00-0e49-11ea-9d6a-fb457f67a59a.png)

Take care of the polarity to avoid drama. Each LED need around 50mA full brightness, so 3A max for 60 LEDs and 7.2A max for 144 LEDS !!! So USB battery bank are enough and simple for 60 LEDs, but not for 144 LED. In any case, check the output amps of your battery bank and ajust the brightness wisely.

### Wifi

```
//#define STA       // Decomment this to use STA mode instead of AP
```

ImagePainting can operate in two separate mode :

* **STA :** As STAtion. In this case, the ESP8266 will connect to your router and be another client of your network. You have to look at the serial monitor of the Arduino IDE during the boot of the ESP8266 to find his IP address. This mode is mainly not interesting, except for debug.

* **AP :** As Access Point. In this case, the ESP8266 will provide his own wifi network and you will connect directly on him. To keep it simple, there is no password and the DNS will route you directly on the right IP. But if there is a problem, go to http://192.168.1.1 to reach your ESP8266. This is the default mode of ImagePainting as it is very unlikely to have a wifi router everywhere.

```
// WIFI --------------
#ifdef STA // STA Mode
const char* ssid = "Moto C Plus 1105"; // your wifi ssid for STA mode
const char* password = "12345678"; // your wifi password for AP mode
#else // AP Mode
const char* ssid = "imagePainting"; // wifi ssid for AP mode
IPAddress apIP(192, 168, 1, 1); // wifi IP for AP mode
#endif
// end WIFI -----------
```

* **SSID and PASSWORD :** you have to set your router ssid and password if you use STA mode.

### DNS

```
//#define DNS       // Decomment this to use DNS
```
DNS can be usefull in AP mode to retrieve easily the webpage when connected to the ESP8266. In my test, it's not so efficient on android as notification use a browser that not very html5 compatible. It don't hurt the code so i let it as an option, but i don't use it anymore.

## Software Prerequisites And Installing

### Arduino IDE

You need to install the following board/tool/library :

* [ESP8266](https://github.com/esp8266/Arduino) - The ESP8266 core for Arduino

* [arduino-esp8266littlefs-plugin](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases) - The Arduino plugin to upload LittleFS filesystems to ESP8266
 
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - The Adafruit enhanced NeoPixel support library

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - The JSON library for Arduino

### Flash instruction

* Tweak the code for your hardware and wifi.

* Tweak the ESP8266 to max out its possibility. For exemple, for the Wemos D1, the CPU can be set to 160Mhz and the FS to 3MB.

* Flash ImagePainting from the Arduino IDE.

* Upload the data by clicking to "ESP8266 LittleFS Data Upload". 

### Install the app

The compile app (currently for android) can be find in /android/release. This app is compile for AP use as the http://192.168.1.1 is hardcoded. If you need to permanently change this IP (or you don't trust me), you can compile the source with [PhoneGap](https://phonegap.com/).

## Use ImagePainting

I will explain the use from the android app as it is the recommand way to use ImagePainting. I consider the webpage as legacy and i strongly recommand the app. You can delete index.html and title.png from the data file, if you plan to use the app.

### Actions screen

![action](https://user-images.githubusercontent.com/2498942/71197104-300d4600-2291-11ea-9ccc-bb8528c9db1d.png)

* **File :** Select a Bitmap on the ESP8266 flash memory.
  * **Start :** Select the beginning of your animation
  * **Stop :** Select the end of your animation

* **Options :**
  * **Duration :** Show the estimated duration of the animation. If you change the duration, it will change the delay to fit your need.

* **Actions :**
  * **Light :** Set the LED Strip to **Color** to use it as a flashlight or for special effect. Stop your runnig animation.
  * **Burn :** Set the LED Strip to the first line of the Bitmap for special effect. Stop your animation.
  * **Play :** Play or pause your running animation
  * **Stop :** Turn off your LED Strip. Stop your running animation.

### Settings screen

![settings](https://user-images.githubusercontent.com/2498942/71197106-30a5dc80-2291-11ea-9fb8-94fa9e1a0d1c.png)

* **Delay :** Time between two frames in ms.
* **Brightness :** Brightness of the LED Strip. Value possible 0 (black) to 255 (full).

* **Countdown :** Time in ms before launching the animation. This value is use by this checkbox :
  * **Countdown ?** [x] The animation is start after **Countdown** ms.

* **Repeat :** Number of times the Bitmap is animate. This value is use by this three checkboxes :
  * **Invert ?** [x] The animation start with the end of the Bitmap
  * **Repeat ?** [x] Activate the repetition according to **Repeat**
  * **Bounce ?** [x] Activate the bounce (normal-invert-normal...etc) repetition according to **Repeat**

* **Pause :** Number of line between two pause during the animation. This value is use by this two checkboxes :
  * **Pause?** [x] The animation is pause each **Pause** line(s)
  * **Cut?** [x] The animation is cut each **Pause** line(s)

* **Color :** Color use by **Light** and **Endcolor ?**. This value is use by this two checkboxes :
  * **Endoff ?** [x] Turn off the LED Strip when the animation is pause/stop or end. Usefull when the Bitmap doesn't end with a black line.
  * **Endcolor?** [x] Set the LED Strip to **Color** when the animation is pause/stop or end. Usefull for special effect.

* **Actions :**
  * **Save :** Save current settings to the ESP8266. This allow you to Restore your settings after a shutdown.
  * **Restore :** Restore current settings from the ESP8266.

### Upload screen

![upload](https://user-images.githubusercontent.com/2498942/71197108-313e7300-2291-11ea-9ac7-f169fd2a0623.png)

* **File :** Select a file on your smartphone. Preview is for image only.

* **Options :**
  * **Gamma :** Apply Gamma to the Bitmap which will appear darker on the smartphone than on the LED strip.
  * **Bottom to top ?** [x] Change animation side from "Left to Right" to "Bottom to Top". Usefull for vertical Bitmap.
  * **Pixels :** Change the number of pixels. The Bitmap is put in the center of the LED strip.
  * **Line Cut :** Cut horizontal black line of the specify width in the Bitmap.

* **Actions :**
  * **Upload Original :** Upload directly selected file to the ESP8266.
  * **Upload Convert :** Convert and Upload selected image file to the ESP8266. 
  * **Download Convert :** Convert and Download selected image file to your smartphone.

### Download screen

![download](https://user-images.githubusercontent.com/2498942/71197105-30a5dc80-2291-11ea-9fb0-32700abd8e69.png)

* **File :** Select a file on the ESP8266 flash memory. Preview is for bitmap only.
* **Actions :**
  * **Download :** Download selected file to your smartphone.
  * **Delete :** Delete selected file from ESP8266. "welcome.bmp" can't be erase as system files.

### System screen

![system](https://user-images.githubusercontent.com/2498942/71197107-313e7300-2291-11ea-9b06-2613c42608b5.png)

* **ESP8266 FILSYSTEM :** Memory use on the ESP8266.
* **ESP8266 ADDRESS :** Address of the ESP8266.
* **APPLICATION THEME :** Choose between Dark or Light theme.

## Case scenario

I try my best to show you some possibility of ImagePainting.

![help1](https://user-images.githubusercontent.com/2498942/69259671-e57ba980-0bbe-11ea-8891-45a0cb5b84b9.png)

![help2](https://user-images.githubusercontent.com/2498942/68999791-1362a480-08c6-11ea-9d93-aec3192cc787.png)

## About Picture

ESP8266 is not powerful enough to handle compress format as jpeg, png... etc. So yours pictures has to follow this organisation.

![bitmapExplanation](https://user-images.githubusercontent.com/2498942/68552286-a3f83b00-0415-11ea-8ec8-e8d4f2450843.jpg)

* Only Bitmap with 24 bits per pixel (or 8 bits per color) are supported. If there is a problem with such a Bitmap, try to save it in bmp3 format.
* Your Bitmap must be rotate by 90°. ImagePainting will display Bitmap line by line from the left to the right. Also don't be fool by the render in the webpage, ImagePainting rotate your Bitmap back.
* The width of your Bitmap has to be the same than the length of your LED strip or your Bitmap will be crop.

Hopefully, the use of the app build in convert tool free you of all this consideration.

## Bitmap Pack

I have put you resize version of the original Bitmap pack from [pixelstick](http://www.thepixelstick.com/index.html). This Bitmap pack come originaly at 200px wide and i resize it at 60px and 144px to fit standard LED strip.

![help8](https://user-images.githubusercontent.com/2498942/69486401-9e561880-0e4b-11ea-8fd5-3fc7776f90fc.png)

Thanks [pixelstick](http://www.thepixelstick.com/index.html) for this nice Bitmap pack.