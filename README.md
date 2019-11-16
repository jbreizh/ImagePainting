# imagePaintingSTA

Yet another project on LightPainting .... WAIT !!!

Existing project on the topic like Adafruit [NeoPixel Painter](https://learn.adafruit.com/neopixel-painter/overview) or [Light Painting with Raspberry Pi](https://learn.adafruit.com/light-painting-with-raspberry-pi/overview) are great and have been a great source of inspiration/information, but i find those too complicated and unpractical on the field. You need too much hardware and you can't be far from your computer to load other image or tweet parameters.

My project is base on hardware and use simplicity. All you need is an USB battery bank, a Led strip, an ESP8266 and a webbrowser (computer of course but mainly smartphone).

After a simple hardware assembly and flashing my code, all actions (image upload/delete, start, pause, stop, light) and parameters (delay, brightness, repeat, bounce, invert) are send through your webbrowser. So load your smartphone with your picture and you are ready to shoot.

### Hardware Prerequisites

![hardware](https://user-images.githubusercontent.com/2498942/68552594-e7a07400-0418-11ea-9d8a-11a54ea146d7.jpg)

* **USB Battery bank :** I use sony CP-V3A for 60 Leds. Take care of the output amps and use regular USB.

* **Board :** I use Wemos D1 mini clone, but i should work on other ESP8266 base board like NodeMCU.

* **Led :** I use APA102. It could work on other led like WS2812, but APA102 are better and faster.

### Hardware Assembly

* **For software bit bang :** My sketch use pin D1 for DATA and pin D2 for CLOCK, but you can change it.

![help4](https://user-images.githubusercontent.com/2498942/69000175-d00b3480-08cb-11ea-8f84-812b48917d14.png "Hardware assembly for software bit bang")

### Software Prerequisites And Installing

**Arduino IDE with the following board/tool/library installed :**
* [ESP8266](https://github.com/esp8266/Arduino) - The ESP8266 core for Arduino
* [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin) - The Arduino plugin for uploading files to ESP8266 file system
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - The Adafruit enhanced NeoPixel support library
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - The JSON library for Arduino

**Tweet the code to fit your need :**
```
// APA102 --------------
const int NUMPIXELS = 60;
uint8_t BRIGHTNESS = 40;
const int DATA_PIN = D1; //GREEN
const int CLOCK_PIN = D2; //Yellow
NeoPixelBus<DotStarBgrFeature, DotStarMethod> STRIP(NUMPIXELS, CLOCK_PIN, DATA_PIN); // for software bit bang
//NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod> STRIP(NUMPIXELS); // for hardware SPI : CLOCK_PIN : D5 Yellow / DATA_PIN : D7 GREEN
// end APA102-----------

// WIFI --------------
ESP8266WebServer server;
const char* ssid = "Moto C Plus 1105";
const char* password = "12345678";
// end WIFI-----------
```
* **NUMPIXELS :** Number of pixel in your led strip
* **DATA_PIN :** Pin for data in software bit bang mode
* **CLOCK_PIN :** Pin for clock in software bit bang mode
* **ssid :** your router ssid
* **password :** your router password

**Flash the code and upload data to SPIFFS. The end....**

## Use

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
- [x] Invert : The animation start with the end of the Bitmap

### Manage your option

* **Delay :** Time between two frames in ms. Value possible 0ms to 255ms, but no garantee under 10ms
* **Brightness :** Brightness of the LED. Value possible 0 (black) to 255 (full)

* **Repeat :** Number of times the Bitmap is animate. Value possible 0 x (0 time so 1 animation) to 255 x (255 times so 256 animations). This value is use by this two chexboxes :
- [x] Repeat? : Activate the repetition according to **Repeat**
- [x] Bounce?: Activate the bounce (normal-invert-normal...etc) repetition according to **Repeat**

* **Pause :** Number of pixels between two pause during the animation. Value possible 0 px (0 px between pause so no pause) to 255 (255 px between pause so 255 lines animate the 255 line pause ...etc). This value is use by this two chexboxes :
- [x] Pause? : 
- [x] Cut? : 

* **Color :** Color use by **Light** and **[x] Endcolor?**
- [x] Endoff? : Turn off the LED when the animation is pause/stop or end. Usefull when the Bitmap doesn't end with a black line
- [x] Endcolor? : Set all the LED to **Color** when the animation is pause/stop or end. Usefull for special effect.
### Manage your action

* **Light :** Set all the LED to **Color** to use it as a flashlight or for special effect. Stop your animation.
* **Burn :** Set all the LED to the first line of the Bitmap for special effect. Stop your animation.
* **Play :** Play or pause your animation
* **Stop :** Turn off your LED. Stop your animation.

## Graphic explanation

I try my best to show you some possibility of Imagepainting.

![help1](https://user-images.githubusercontent.com/2498942/68999790-1362a480-08c6-11ea-976c-e6bd6f43f333.png)
![help2](https://user-images.githubusercontent.com/2498942/68999791-1362a480-08c6-11ea-9d93-aec3192cc787.png)

## About Picture

![bitmapExplanation](https://user-images.githubusercontent.com/2498942/68552286-a3f83b00-0415-11ea-8ec8-e8d4f2450843.jpg)

ESP8266 is not powerful enough to handle compress format as jpeg, png... etc. So yours pictures has to follow this organisation :

* Only Bitmap with 24 bits per pixel (or 8 bits per color) are supported. If there is a problem with such a Bitmap, try to save it in bmp3 format.
* The Bitmap must be rotate by 90°. Imagepainting will display the Bitmap line by line from the left to the right. Also don't be fool by the render in the webpage, Imagepainting rotate your Bitmap back.
* The width of your Bitmap has to be the same than the length of your LED strip or your Bitmap will be crop.
