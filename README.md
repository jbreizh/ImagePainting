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

* **Neopixels :** FEATURE : Neo***Feature and METHOD : Neo800KbpsMethod. You must use RDX0/GPIO3 PIN (label RX on Wemos D1 mini) as specify [here](https://github.com/Makuna/NeoPixelBus/wiki/ESP8266-NeoMethods#neoesp8266dma800kbpsmethod).


![help5](https://user-images.githubusercontent.com/2498942/69486235-0c99db80-0e4a-11ea-9101-3f5566b0bd12.png)

* **Dotstars :** FEATURE : DotStar***Feature and METHOD : DotStarSpiMethod. You must use the hardware SPI pin from your board (DATA = MOSI and CLOCK = SCLK). For the Wemos D1 mini MOSI is D7 and SCLK is D5.

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
//#define STA       // Decomment this to use ImagePainting in STA mode and setup your ssid/password
```

ImagePainting can operate in two separate mode :

* **STA :** As STAtion. In this case, the ESP8266 will connect to your router and be another client of your network. You have to look at the serial monitor of the Arduino IDE during the boot of the ESP8266 to find his IP address. This mode is mainly not interesting, except for debug.

* **AP :** As Access Point. In this case, the ESP8266 will provide his own wifi network and you will connect directly on him. To keep it simple, there is no password and the DNS will route you directly on the right IP. But if there is a problem, go to http://192.168.1.1 to reach your ESP8266. This is the default mode of ImagePainting as it is very unlikely to have a wifi router everywhere.

```
// WIFI/ DNS --------------
ESP8266WebServer server;
#ifdef STA // STA Mode
const char* ssid = "Moto C Plus 1105"; // your wifi ssid for STA mode
const char* password = "12345678"; // your wifi password for STA mode
#else // AP Mode
const char* ssid = "imagePainting"; // wifi ssid for AP mode
IPAddress apIP(192, 168, 1, 1); // wifi IP for AP mode
DNSServer dnsServer;
const byte DNS_PORT = 53;
#endif
// end WIFI/DNS -----------
```

* **STA :** you have to set your router ssid and password.

* **AP :** you can change the ssid and ip of the ESP8266, but what the point...

## Software Prerequisites And Installing

### Arduino IDE

You need to install the following board/tool/library :

* [ESP8266](https://github.com/esp8266/Arduino) - The ESP8266 core for Arduino

* [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin) - The Arduino plugin for uploading
 files to ESP8266 file system
 
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - The Adafruit enhanced NeoPixel support library

* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - The JSON library for Arduino

### Flash instruction

Tweak the code for your hardware and wifi and flash ImagePainting from the Arduino IDE. Don't forget to upload the data by clicking to "ESP8266 Sketch Data Upload". You can also tweak the SPIFFS to have more space for yours Bitmaps.

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

![help1](https://user-images.githubusercontent.com/2498942/69259671-e57ba980-0bbe-11ea-8891-45a0cb5b84b9.png)

![help2](https://user-images.githubusercontent.com/2498942/68999791-1362a480-08c6-11ea-9d93-aec3192cc787.png)

## About Picture

ESP8266 is not powerful enough to handle compress format as jpeg, png... etc. So yours pictures has to follow this organisation.

![bitmapExplanation](https://user-images.githubusercontent.com/2498942/68552286-a3f83b00-0415-11ea-8ec8-e8d4f2450843.jpg)

* Only Bitmap with 24 bits per pixel (or 8 bits per color) are supported. If there is a problem with such a Bitmap, try to save it in bmp3 format.
* Your Bitmap must be rotate by 90°. ImagePainting will display Bitmap line by line from the left to the right. Also don't be fool by the render in the webpage, ImagePainting rotate your Bitmap back.
* The width of your Bitmap has to be the same than the length of your LED strip or your Bitmap will be crop.

## Bitmap Pack

I have put you resize version of the original Bitmap pack from [pixelstick](http://www.thepixelstick.com/index.html). This Bitmap pack come originaly at 200px wide and i resize it at 60px and 144px to fit standard LED strip.

![help8](https://user-images.githubusercontent.com/2498942/69486401-9e561880-0e4b-11ea-8fd5-3fc7776f90fc.png)

Thanks [pixelstick](http://www.thepixelstick.com/index.html) for this nice Bitmap pack.
