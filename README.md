# imagePaintingSTA

Yet another project on LightPainting ....

Existing project on the topic like Adafruit [NeoPixel Painter](https://learn.adafruit.com/neopixel-painter/overview) or [Light Painting with Raspberry Pi](https://learn.adafruit.com/light-painting-with-raspberry-pi/overview) are great and have been a great source of inspiration/information, but i find those to complicated and unpractical on the field. You need to much hardware and you can't be far from your computer to load other image or tweet parameters.

My project is base on hardware and use simplicity. All you need is an USB battery bank, a Led strip, an ESP8266 and a webbrowser (computer of course but mainly smartphone).

After a simple hardware assembly and flashing my code, all actions (image upload/delete, start, pause, stop, light) and parameters (delay, brightness, repeat, bounce, invert) are send through your webbrowser. So load your smartphone with your picture and you are ready to shoot.

### Hardware Prerequisites

* **USB Battery bank:** I use sony CP-V3A for 60 Leds. Take care of the output amps and use regular USB.

* **Board :** I use Wemos D1 mini clone, but i should work on other ESP8266 base board like NodeMCU.

* **Led :** I use APA102. It could work on other led like WS2812, but APA102 are better and faster.

* **Webbrowser :** All modern webbrowser should be good to handle my poor coding and if you read this, you are good ;-)

* **Hardware assembly :**

schematic

### Software Prerequisites

Arduino IDE with the following board/tool/library installed :
* [ESP8266](https://github.com/esp8266/Arduino) - The ESP8266 core for Arduino
* [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin) - The Arduino plugin for uploading files to ESP8266 file system
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - The Adafruit enhanced NeoPixel support library
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson) - The JSON library for Arduino

### Installing

A step by step series of examples that tell you how to get a development env running

Say what the step will be

```
Give the example
```

## Use

Explain how to run the automated tests for this system

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* Hat tip to anyone whose code was used
* Inspiration
* etc
