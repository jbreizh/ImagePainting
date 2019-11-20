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

This is my hardware assembly for the Wemos D1 mini for software SPI:

![help4](https://user-images.githubusercontent.com/2498942/69257223-ffb38880-0bba-11ea-9134-2ce36ff309a4.png" Hardware assembly for software bit bang")

This hardware is 100% working for me, but could be improve by adding a capacitor to filter the power and a level schifter to convert the 3.3v logic to a 5v for the led. The best option will be this shield [Hex Wemos D1 Mini Wi-Fi LED Controller](https://www.evilgeniuslabs.org/hex-wemos-d1-mini-wifi-led-controller#assembly-instructions) from Evil Genius.

### LED Strip connection

APA102 are 4 wires SPI driven LED (+5V , DATA, CLOCK , GROUND). This section of the code define the LED strip :

```
// APA102 --------------
const int NUMPIXELS = 60;
uint8_t BRIGHTNESS = 40;
const int DATA_PIN = D1;
const int CLOCK_PIN = D2;
NeoPixelBus<DotStarBgrFeature, DotStarMethod> STRIP(NUMPIXELS, CLOCK_PIN, DATA_PIN); // for software bit bang: CLOCK_PIN : D2 Yellow / DATA_PIN : D1 GREEN
//NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod> STRIP(NUMPIXELS); // for hardware SPI : CLOCK_PIN : D5 Yellow / DATA_PIN : D7 GREEN
//NeoPixelBus<DotStarBgrFeature, DotStarSpi2MhzMethod> STRIP(NUMPIXELS); // for hardware SPI : CLOCK_PIN : D5 Yellow / DATA_PIN : D7 GREEN
// end APA102-----------
```

For the ESP8266, there is 2 way to produce the signal for DATA and CLOCK :

* **Software SPI :** I use pin D1 for DATA and pin D2 for CLOCK, but you can change it.

* **Hardware SPI :** You must use the hardware SPI pin from your board (DATA = MOSI and CLOCK = SCLK). For the Wemos D1 mini MOSI is D7 and SCLK is D5. The Hardware SPI is faster than the Software method and will allow a faster refresh time.

### Push button connection

The 2 push buttons are optionals. ImagePainting always work from the webpage and you still need your smartphone to upload Bitmap or tweak parameters. However, it's more convenient to launch your animation with a button than clicking on a touch screen.

By defaut, ImagePainting handle the 2 push buttons (BTNA and BTNB) like this :

* **BTNA :** "short click" --> Play/Pause and "long click" --> Burn

* **BTNB :** "short click" --> Stop and "long click" --> Light

This section of the code define the push button :

```
// BUTTON --------------
long DEBOUNCETIME = 50; //Debouncing Time in Milliseconds
long HOLDTIME = 500; // Hold Time in Milliseconds
const int BTNA_PIN = D3;
long BTNATIMER = 0;
boolean ISBTNA = false;
boolean ISBTNAHOLD = false;
const int BTNB_PIN = D4;
long BTNBTIMER = 0;
boolean ISBTNB = false;
boolean ISBTNBHOLD = false;
// end BUTTON-----------
```

* **Change the PIN :** I use D3 and D4 on purpose for my Wemos D1. This is internal Pull-Up Pin, so check the specification of your board before changing those PIN.

* **HOLDTIME :** HOLDTIME can be tweak to have a shorter/longer "long click".

* **DEBOUNCETIME :** DEBOUNCETIME can be tweak to have a good button response without false detection.

## Software Prerequisites And Installing

### Arduino IDE

You need to install the following board/tool/library :

* [ESP8266](https://github.com/esp8266/Arduino) - The ESP8266 core for Arduino

* [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin) - The Arduino plugin for uploading
 files to ESP8266 file system
 
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - The Adafruit enhanced NeoPixel support library

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - The JSON library for Arduino

### Wifi

```
// WIFI --------------
ESP8266WebServer server;
const char* ssid = "Moto C Plus 1105";
const char* password = "12345678";
// end WIFI-----------
```

* **ssid :** your router ssid
* **password :** your router password

## Use ImagePainting

This is the webpage that your ESP8266 serve you :

![help3](https://user-images.githubusercontent.com/2498942/68999792-13fb3b00-08c6-11ea-9304-21183b95ae0f.png)

### Manage your file

* **Delete :** Select a file on the ESP8266 flash memory to delete it. "error.bmp" and "welcome.bmp" can't be erase as they are system files.
* **File :** Select a Bitmap and upload it on the ESP8266 flash memory. Keep in mind that SPIFFS is just 1 or 2 MB, so it's not for 4K streaming ;-)

### Manage your picture

* **Image :** Select a Bitmap on the ESP8266 flash memory to animate it. In case of problem with the file (see the **About Picture**), you will be send to "error.bmp"
* **Write :** A cryptic but yet very important button. It set all your option in the ESP8266. So each time you modify something, hit "Write". It never hurt, but you can't write when there is a running or pause animation.
* **Start :** Select the beginning of your animation (it will be keep when repeat or bounce)
* **Stop :** Select the end of your animation (it will be keep when repeat or bounce)
  * **Invert ?** [x] The animation start with the end of the Bitmap

### Manage your options

* **Delay :** Time between two frames in ms. Value possible 0ms to 255ms, but no garantee under 10ms
* **Brightness :** Brightness of the LED Strip. Value possible 0 (black) to 255 (full)

* **Repeat :** Number of times the Bitmap is animate. Value possible 0 x (0 time so 1 animation) to 255 x (255 times so 256 animations). This value is use by this two chexboxes :
  * **Repeat ?** [x] Activate the repetition according to **Repeat**
  * **Bounce ?** [x] Activate the bounce (normal-invert-normal...etc) repetition according to **Repeat**

* **Pause :** Number of line between two pause during the animation. Value possible 0 px (0 px between pause so no pause) to 255 (255 px between pause so 255 lines animate the 255 line pause ...etc). This value is use by this two chexboxes :
  * **Pause?** [x] The animation is pause each **Pause** line(s)
  * **Cut?** [x] The animation is cut each **Pause** line(s)

* **Color :** Color use by **Light** and **Endcolor ?**
  * **Endoff ?** [x] Turn off the LED Strip when the animation is pause/stop or end. Usefull when the Bitmap doesn't end with a black line
  * **Endcolor?** [x] Set the LED Strip to **Color** when the animation is pause/stop or end. Usefull for special effect.
### Manage your action

* **Light :** Set the LED Strip to **Color** to use it as a flashlight or for special effect. Stop your runnig animation.
* **Burn :** Set the LED Strip to the first line of the Bitmap for special effect. Stop your animation.
* **Play :** Play from **Start** or pause your running animation
* **Stop :** Turn off your LED Strip. Stop your running animation.

## Case scenario

I try my best to show you some possibility of ImagePainting.

![help1](https://user-images.githubusercontent.com/2498942/68999790-1362a480-08c6-11ea-976c-e6bd6f43f333.png)
![help2](https://user-images.githubusercontent.com/2498942/68999791-1362a480-08c6-11ea-9d93-aec3192cc787.png)

## About Picture

![bitmapExplanation](https://user-images.githubusercontent.com/2498942/68552286-a3f83b00-0415-11ea-8ec8-e8d4f2450843.jpg)

ESP8266 is not powerful enough to handle compress format as jpeg, png... etc. So yours pictures has to follow this organisation :

* Only Bitmap with 24 bits per pixel (or 8 bits per color) are supported. If there is a problem with such a Bitmap, try to save it in bmp3 format.
* Your Bitmap must be rotate by 90°. ImagePainting will display Bitmap line by line from the left to the right. Also don't be fool by the render in the webpage, ImagePainting rotate your Bitmap back.
* The width of your Bitmap has to be the same than the length of your LED strip or your Bitmap will be crop.

## Picture Pack

I have put you resize version of the original Bitmap pack from [pixelstick](http://www.thepixelstick.com/index.html). This Bitmap pack come originaly at 200px wide and i resize it at 60px and 144px to fit standard LED strip.
Thanks [pixelstick](http://www.thepixelstick.com/index.html) for this nice Bitmap pack.
