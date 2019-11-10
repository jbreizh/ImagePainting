# imagePaintingSTA

Yet another project on LightPainting ....

Existing project on the topic like Adafruit [NeoPixel Painter](https://learn.adafruit.com/neopixel-painter/overview) or [Light Painting with Raspberry Pi](https://learn.adafruit.com/light-painting-with-raspberry-pi/overview) are great and have been a great source of inspiration/information, but i find those to complicated and unpractical on the field. You need to much hardware and you can't be far from your computer to load other image or tweet parameters.

My project is base on hardware and use simplicity. All you need is an USB battery bank, a Led strip, an ESP8266 and a webbrowser (computer of course but mainly smartphone).

After a simple hardware assembly and flashing my code, all actions (image upload/delete, start, pause, stop, light) and parameters (delay, brightness, repeat, bounce, invert) are send through your webbrowser. So load your smartphone with your picture and you are ready to shoot.

### Hardware Prerequisites

* **USB Battery bank :** I use sony CP-V3A for 60 Leds. Take care of the output amps and use regular USB.

* **Board :** I use Wemos D1 mini clone, but i should work on other ESP8266 base board like NodeMCU.

* **Led :** I use APA102. It could work on other led like WS2812, but APA102 are better and faster.

* **Webbrowser :** All modern webbrowser should be good to handle my poor coding and if you read this, you are good ;-)

* **Hardware Assembly :**
![software](https://user-images.githubusercontent.com/2498942/68550264-a05ab900-0401-11ea-81c8-7a4c1f4a8635.png "Hardware assembly for software bit bang")
![hardware](https://user-images.githubusercontent.com/2498942/68550268-af416b80-0401-11ea-9a1b-4eb83e9c52c5.png "Hardware assembly for hardware SPI")

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

![controleScreen](https://user-images.githubusercontent.com/2498942/68551545-3eed1700-040e-11ea-8ee4-be0f984f540b.jpg)

* **File :** Select a picture and upload it on the ESP8266 flash memory (see the **About Picture**)
* **Image :** Select a picture on the ESP8266 flash memory to animate it or to delete it
* **Delay :** Time between two frames in ms. Value possible 0ms to 255ms, but no garantee under 10ms
* **Brightness :** Brightness of the LED. Value possible 0 (black) to 255 (full)
* **Repeat :** Number of times the picture is animate. Value possible 0 (0 repetition so 1 animation) to 255 (255 repetitions so 256 animations)
* **Option :**
- [x] Repeat : Activate the repetition according the number of times set in **Repeat :**
- [x] Bounce : Activate the bounce repetition according the number of times set in **Repeat :**
- [x] Invert : The animation start with the end of the picture
- [x] Endoff : Turn off the LED when the animation is pause/stop or end. Usefull when the picture doesn't end with a black line
* **Light :** Turn on your LED in white to use it as a flashlight. Usefull in dark situation ;-)
* **Play :** Play or pause your animation
* **Stop :** Turn off your LED. Stop your animation.

## About Picture

Due to hardware limitation of the ESP8266, imagePainting only animate Bitmap 24 bits/pixel rotate by 90°.

![bitmapExplanation](https://user-images.githubusercontent.com/2498942/68552286-a3f83b00-0415-11ea-8ec8-e8d4f2450843.jpg)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details
