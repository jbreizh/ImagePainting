#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <SPI.h>
#include <FS.h>

// APA102 --------------
const int NUMPIXELS = 60;
uint8_t BRIGHTNESS = 40;
const int DATA_PIN = D1;
const int CLOCK_PIN = D2;
NeoPixelBus<DotStarBgrFeature, DotStarMethod> STRIP(NUMPIXELS, CLOCK_PIN, DATA_PIN); // for software bit bang
//NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod> STRIP(NUMPIXELS); // for hardware SPI
// end APA102-----------

// WIFI --------------
ESP8266WebServer server;
char* ssid = "Moto C Plus 1105";
char* password = "12345678";
// end WIFI-----------

// FS --------------
fs::File fsUploadFile; // hold uploaded file
// end FS -----------

// BITMAP --------------
fs::File BMPFILE; // hold bitmap file
NeoBitmapFile<DotStarBgrFeature, fs::File> NEOBMPFILE;
// end BITMAP -----------

// ANIMATION --------------
NeoPixelAnimator ANIMATIONS(1); // NeoPixel animation management object
uint16_t ANIMATIONSTATE;
// end ANIMATION --------------

// RUNTIME --------------
uint8_t DELAY = 100;
uint8_t REPEAT = 0; uint8_t REPEATCOUNTER;
bool ENDOFF = false;
bool INVERT = false;
bool BOUNCE = false;

// end RUNTIME --------------

//SHADER --------------
const RgbColor Black(0);
template<typename T_COLOR_FEATURE> class BrightnessShader : public NeoShaderBase
{
  public:
    BrightnessShader():
      NeoShaderBase(),
      _brightness(255) // default to full bright
    {}

    void Apply(uint16_t index, uint8_t* pDest, uint8_t* pSrc)
    {
      // we don't care what the index is so we ignore it
      //
      // to apply our brightness shader,
      // use the source color, modify, and apply to the destination

      // for every byte in the pixel,
      // scale the source value by the brightness and
      // store it in the destination byte
      const uint8_t* pSrcEnd = pSrc + T_COLOR_FEATURE::PixelSize;
      while (pSrc != pSrcEnd)
      {
        *pDest++ = (*pSrc++ * (uint16_t(_brightness) + 1)) >> 8;
      }
    }

    // provide an accessor to set brightness
    void setBrightness(uint8_t brightness)
    {
      _brightness = brightness;
      Dirty(); // must call dirty when a property changes
    }

    // provide an accessor to get brightness
    uint8_t getBrightness()
    {
      return _brightness;
    }

  private:
    uint8_t _brightness;
};

// create an instance of our shader object with the same feature as our buffer
BrightnessShader<DotStarBgrFeature> SHADER;
// end SHADER --------------

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void setup()
{
  // FS setup
  SPIFFS.begin();

  // Serial setup
  Serial.begin(115200);

  // Wifi setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Webserver setup
  // list available files
  server.on("/list", HTTP_GET, handleFileList);

  // delete file
  server.on("/delete", HTTP_DELETE, handleFileDelete);

  // handle file upload
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "SUCCESS");
  }, handleFileUpload);

  // handle bitmap Load
  server.on("/bitmapLoad", HTTP_POST, handleBitmapLoad);

  // handle bitmap Play
  server.on("/bitmapPlayPause", HTTP_GET, handleBitmapPlayPause);

  // handle bitmap Stop
  server.on("/bitmapStop", HTTP_GET, handleBitmapStop);

  // handle light
  server.on("/light", HTTP_GET, handleLight);

  // handle parameter Read
  server.on("/parameterRead", HTTP_GET, handleParameterRead);

  // handle parameter Write
  server.on("/parameterWrite", HTTP_POST, handleParameterWrite);

  // called when the url is not defined
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "404: Not found");
    }
  });

  // Webserver start
  server.begin();

  // LED setup
  SHADER.setBrightness(BRIGHTNESS);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void loop()
{
  server.handleClient();
  ANIMATIONS.UpdateAnimations();
  STRIP.Show();
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
String getContentType(String filename)
{
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".bmp")) return "image/bmp";
  return "text/plain";
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterRead()
{
  // New json document
  StaticJsonDocument<200> jsonDoc;

  // Store parameter in json document
  jsonDoc["delay"] = DELAY;
  jsonDoc["brightness"] = BRIGHTNESS;
  jsonDoc["repeat"] = REPEAT;
  jsonDoc["endoff"] = ENDOFF;

  // convert json document to String
  String msg = "";
  serializeJson(jsonDoc, msg);

  // Parameter are read
  server.send(200, "text/html", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterWrite()
{
  ENDOFF = false;
  Serial.print("ENDOFF :");
  Serial.println(ENDOFF);

  Serial.print("JSON :");
  Serial.println(server.arg("plain"));

  // New json document
  StaticJsonDocument<200> jsonDoc;

  // Convert json String to json object
  DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));

  // Check if the json is right
  if (error)
  {
    String msg = "WRITE ERROR : ";
    msg += error.c_str();
    return server.send(500, "text/plain", msg);
  }

  // Write parameters in ESP8266
  DELAY = jsonDoc["delay"];
  BRIGHTNESS = jsonDoc["brightness"];
  REPEAT = jsonDoc["repeat"];
  ENDOFF = jsonDoc["endoff"];

  Serial.print("ENDOFF :");
  Serial.println(ENDOFF);

  // Parameter are write
  server.send(200, "text/html", "WRITE SUCCESS");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileDelete()
{
  // parse parameter from request
  String path = server.arg("file");

  // make sure we get a file name as a URL argument and protect root path
  if (path == "" || path == "/") return server.send(500, "text/plain", "BAD ARGS");

  // check if the file exists
  if (!SPIFFS.exists(path)) return server.send(404, "text/plain", "FILE NOT FOUND!");

  // Delete the file
  SPIFFS.remove(path);

  // File is delete
  String msg = "DELETE FILE : " + path;
  server.send(200, "text/plain", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool handleFileRead(String path)
{
  // Serve index file when top root path is accessed
  if (path.endsWith("/")) path += "index.html";

  // Check if the file exists
  if (!SPIFFS.exists(path)) return false;

  // Open the file
  fs::File file = SPIFFS.open(path, "r");

  // Display the file on the client's browser
  server.streamFile(file, getContentType(path));

  // Close the file
  file.close();
  return true;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileUpload()
{
  //
  HTTPUpload& upload = server.upload();

  //
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/"))
      filename = "/" + filename;

    fsUploadFile = SPIFFS.open(filename, "w");
  }

  //
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  }

  //
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (fsUploadFile)
      fsUploadFile.close();
  }
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileList()
{
  // Assuming there are no subdirectories
  fs::Dir dir = SPIFFS.openDir("/");

  // Scan the files
  String fileList = "";
  while (dir.next())
  {
    // Open the entry
    fs::File entry = dir.openFile("r");

    // Separate by comma if there are multiple files
    if (fileList != "") fileList += ",";

    // Write the entry in the list
    fileList += String(entry.name()).substring(1);

    // Close the entry
    entry.close();
  }

  // FileList is done
  server.send(200, "text/plain", fileList);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleBitmapLoad()
{
  // Close the old bitmap
  BMPFILE.close();

  // Parse parameter from request
  String path = server.arg("file");

  // Make sure we get a file name as a URL argument
  if (path == "") return server.send(500, "text/plain", "BAD ARGS");

  // Check if the file exists
  if (!SPIFFS.exists(path)) return server.send(404, "text/plain", "FILE NOT FOUND");

  // Check if the file is a bitmap
  if (getContentType(path) != "image/bmp") return server.send(500, "text/plain", "WRONG FILE TYPE");

  // Open requested file on SD card
  BMPFILE = SPIFFS.open(path, "r");

  // Check and initialize bitmap from the file
  if (!NEOBMPFILE.Begin(BMPFILE)) return server.send(500, "text/plain", "NOT SUPPORTED BITMAP");

  //Bitmap is load
  String msg = "BITMAP LOAD : Width=" + String(NEOBMPFILE.Width()) + "px Height=" + String(NEOBMPFILE.Height()) + "px";
  server.send(200, "text/plain", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleBitmapPlayPause()
{
  if (ANIMATIONS.IsPaused())
  {
    // Resume animation
    ANIMATIONS.Resume();

    // Paused animation is relaunch
    server.send(200, "text/plain", "PLAY");

  }
  else if (ANIMATIONS.IsAnimationActive(0))
  {
    // Pause animation
    ANIMATIONS.Pause();

    // Blank the strip if needed
    if (ENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));

    // Animation is paused
    server.send(200, "text/plain", "PAUSE");
  }
  else
  {
    // Initialisation
    ANIMATIONSTATE = 0;
    REPEATCOUNTER = REPEAT;

    // Launch a new animation
    ANIMATIONS.StartAnimation(0, DELAY, updateAnimation);

    // New animation is launch
    server.send(200, "text/plain", "PLAY");
  }
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleBitmapStop()
{
  // Stop animation
  ANIMATIONS.StopAnimation(0);

  // Blank the strip
  STRIP.ClearTo(RgbColor(0, 0, 0));

  // Animation is stop
  server.send(200, "text/plain", "STOP");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleLight()
{
  //turn on the strip
  STRIP.ClearTo(RgbColor(255, 255, 255));

  // Strip is turn on
  server.send(200, "text/plain", "LIGHT");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void updateAnimation(const AnimationParam& param)
{
  // Wait for this animation to complete,
  if (param.state == AnimationState_Completed)
  {
    // Pixel to show?
    if (ANIMATIONSTATE < NEOBMPFILE.Height())
    {
      // Restart the animation
      ANIMATIONS.RestartAnimation(param.index);

      // Fil the strip
      NEOBMPFILE.Blt(STRIP, 0, 0, ANIMATIONSTATE, NEOBMPFILE.Width());
      //NEOBMPFILE.Render<BrightnessShader<DotStarBgrFeature>>(STRIP, SHADER, 0, 0, ANIMATIONSTATE, NEOBMPFILE.Width());
      ANIMATIONSTATE += 1;
    }

    // Repetition to do?
    else if (REPEATCOUNTER > 0)
    {
      // Restart the animation
      ANIMATIONS.RestartAnimation(param.index);

      // Initialisation
      REPEATCOUNTER -= 1;
      ANIMATIONSTATE = 0;

      // Fil the strip
      NEOBMPFILE.Blt(STRIP, 0, 0, ANIMATIONSTATE, NEOBMPFILE.Width());
      //NEOBMPFILE.Render<BrightnessShader<DotStarBgrFeature>>(STRIP, SHADER, 0, 0, ANIMATIONSTATE, NEOBMPFILE.Width());
      ANIMATIONSTATE += 1;
    }

    // Nothing more to do
    else
    {
      // Stop the animation
      ANIMATIONS.StopAnimation(param.index);

      // Blank the strip if needed
      if (ENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));
    }
  }
}
