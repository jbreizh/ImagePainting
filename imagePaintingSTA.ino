#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
//#include <SPI.h>
#include <FS.h>

// APA102 --------------
const int NUMPIXELS = 60;
uint8_t BRIGHTNESS = 40;
const int DATA_PIN = D1; //GREEN
const int CLOCK_PIN = D2; //Yellow
NeoPixelBus<DotStarBgrFeature, DotStarMethod> STRIP(NUMPIXELS, CLOCK_PIN, DATA_PIN); // for software bit bang
//NeoPixelBus<DotStarBgrFeature, DotStarSpiMethod> STRIP(NUMPIXELS); // for hardware SPI : CLOCK_PIN : D5 Yellow / DATA_PIN : D7 GREEN
//NeoPixelBus<DotStarBgrFeature, DotStarSpi2MhzMethod> STRIP(NUMPIXELS); // for hardware SPI : CLOCK_PIN : D5 Yellow / DATA_PIN : D7 GREEN
// end APA102-----------

// WIFI --------------
ESP8266WebServer server;
const char* ssid = "Moto C Plus 1105";
const char* password = "12345678";
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
uint16_t ANIMATIONINDEXSTART; uint16_t ANIMATIONINDEX; uint16_t ANIMATIONINDEXSTOP;
// end ANIMATION --------------

// RUNTIME --------------
uint8_t DELAY = 100;
uint8_t REPEAT = 1; uint8_t REPEATCOUNTER;
bool ISREPEAT = false;
bool ISENDOFF = false;
bool ISINVERT = false;
bool ISBOUNCE = false;

// end RUNTIME --------------

//SHADER --------------
template<typename T_COLOR_OBJECT> class BrightnessShader : public NeoShaderBase
{
  public:
    BrightnessShader():
      NeoShaderBase(),
      _brightness(255) // default to full bright
    {}

    T_COLOR_OBJECT Apply(uint16_t index, const T_COLOR_OBJECT src)
    {
      T_COLOR_OBJECT result;

      // below is a fast way to apply brightness to all elements of the color
      // it does assume each element is only 8bits, but this currently is the
      // case
      // This could be replaced with a LinearBlend for safty but is less
      // optimized
      const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(&src);
      uint8_t* pDest = reinterpret_cast<uint8_t*>(&result);
      const uint8_t* pSrcEnd = pSrc + sizeof(T_COLOR_OBJECT);
      while (pSrc != pSrcEnd)
      {
        *pDest++ = (*pSrc++ * (uint16_t(_brightness) + 1)) >> 8;
      }
      return result;
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

// create a non-template type to make it easier to be consistent
typedef BrightnessShader<DotStarBgrFeature::ColorObject> BrightShader;

// create an instance of our shader object with the same feature as our buffer
BrightShader SHADER;
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
    server.send(200, "text/plain", "UPLOAD SUCCESS");
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
  StaticJsonDocument<300> jsonDoc;

  // Store parameter in json document
  jsonDoc["delay"] = DELAY;
  jsonDoc["brightness"] = BRIGHTNESS;
  jsonDoc["repeat"] = REPEAT;
  jsonDoc["isrepeat"] = ISREPEAT;
  jsonDoc["isbounce"] = ISBOUNCE;
  jsonDoc["isinvert"] = ISINVERT;
  jsonDoc["isendoff"] = ISENDOFF;

  // convert json document to String
  String msg = "";
  serializeJson(jsonDoc, msg);

  // Parameter are read
  server.send(200, "text/html", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterWrite()
{
  // New json document
  StaticJsonDocument<300> jsonDoc;

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
  SHADER.setBrightness(BRIGHTNESS);
  REPEAT = jsonDoc["repeat"];
  ISREPEAT = jsonDoc["isrepeat"];
  ISBOUNCE = jsonDoc["isbounce"];
  ISINVERT = jsonDoc["isinvert"];
  ISENDOFF = jsonDoc["isendoff"];

  // Parameter are write
  server.send(200, "text/html", "WRITE SUCCESS");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileDelete()
{
  // parse parameter from request
  String path = server.arg("file");

  // make sure we get a file name as a URL argument and protect root path
  if (path == "" || path == "/") return server.send(500, "text/plain", "DELETE ERROR : BAD ARGS");

  // check if the file exists
  if (!SPIFFS.exists(path)) return server.send(404, "text/plain", "DELETE ERROR : FILE NOT FOUND!");

  // Delete the file
  SPIFFS.remove(path);

  // File is delete
  server.send(200, "text/plain", "DELETE SUCCESS");
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

  // Upload start
  if (upload.status == UPLOAD_FILE_START)
  {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;

  //-------------------------> test for upload?????

    //check if the file already exist
    //if (SPIFFS.exists(filename)) return server.send(415, "text/plain", "UPLOAD ERROR : FILE ALREADY EXIST");

    //check if the file fit in SPIFFS
    //FSInfo fs_info;
    //SPIFFS.info(fs_info);
    //if (upload.totalSize > ) return server.send(413, "text/plain", "UPLOAD ERROR : NOT ENOUGH SPACE");
    
    // Open the file for writing in SPIFFS (create if it doesn't exist)
    fsUploadFile = SPIFFS.open(filename, "w");
  }

  // Upload in progress
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    //Write the received bytes to the file
    if (fsUploadFile) fsUploadFile.write(upload.buf, upload.currentSize);
  }

  // Upload end
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

    // Get the name
    String name = String(entry.name()).substring(1);

    // Hide system file
    if ((name != "index.html") && (name != "error.bmp"))
    {
      // Separate by comma if there are multiple files
      if (fileList != "") fileList += ",";

      // Write the entry in the list
      fileList += name;
    }

    // Close the entry
    entry.close();
  }

  // FileList is done
  server.send(200, "text/plain", fileList);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleBitmapLoad()
{
  // Check for running or paused animation
  if (ANIMATIONS.IsAnimationActive(0) || ANIMATIONS.IsPaused()) return server.send(500, "text/plain", "LOAD ERROR : NOT AVAILABLE");
  
  // Close the old bitmap
  BMPFILE.close();

  //-------------------------> load error.bmp in case of error?????
 
  // Parse parameter from request
  String path = server.arg("file");

  // Make sure we get a file name as a URL argument
  if (path == "") return server.send(500, "text/plain", "LOAD ERROR : BAD ARGS");
  
  // Check if the file exists
  if (!SPIFFS.exists(path)) return server.send(404, "text/plain", "LOAD ERROR : FILE NOT FOUND");
  
  // Check if the file is a bitmap
  if (getContentType(path) != "image/bmp") return server.send(500, "text/plain", "LOAD ERROR : WRONG FILE TYPE");

  // Open requested file on SD card
  BMPFILE = SPIFFS.open(path, "r");

  // Check and initialize bitmap from the file
  if (!NEOBMPFILE.Begin(BMPFILE)) return server.send(500, "text/plain", "LOAD ERROR : NOT SUPPORTED BITMAP");

  // Update the index
  ANIMATIONINDEXSTART = 0;
  ANIMATIONINDEXSTOP = NEOBMPFILE.Height() - 1;

  //Bitmap is load
  String msg = "LOAD SUCCESS : Width=" + String(NEOBMPFILE.Width()) + "px Height=" + String(NEOBMPFILE.Height()) + "px";
  server.send(200, "text/plain", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleBitmapPlayPause()
{
  //--------------> NEOBMPFILE exist ?????????
  // Animation is paused
  if (ANIMATIONS.IsPaused())
  {
    // Resume animation
    ANIMATIONS.Resume();

    // Paused animation is resume
    server.send(200, "text/plain", "RESUME");

  }
  // Animation is active
  else if (ANIMATIONS.IsAnimationActive(0))
  {
    // Pause animation
    ANIMATIONS.Pause();

    // Blank the strip if needed
    if (ISENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));

    // Animation is paused
    server.send(200, "text/plain", "PAUSE");
  }
  // No animation
  else
  {
    // Index
    if (ISINVERT) ANIMATIONINDEX = ANIMATIONINDEXSTOP;
    else ANIMATIONINDEX = ANIMATIONINDEXSTART;

    // Repeat
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
  STRIP.ClearTo(SHADER.Apply(0, RgbColor(255, 255, 255)));

  // Strip is turn on
  server.send(200, "text/plain", "LIGHT");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void updateAnimation(const AnimationParam& param)
{
  // Wait for this animation to complete,
  if (param.state == AnimationState_Completed)
  {
    // ANIMATIONINDEX is in the limit
    if ((ANIMATIONINDEXSTART <= ANIMATIONINDEX) && (ANIMATIONINDEX <= ANIMATIONINDEXSTOP))
    {
      // Restart the animation
      ANIMATIONS.RestartAnimation(param.index);

      // Fil the strip : bitmap is crop to fit the strip !!!
      NEOBMPFILE.Render<BrightShader>(STRIP, SHADER, 0, 0, ANIMATIONINDEX, NEOBMPFILE.Width());

      // Index
      if (ISINVERT) ANIMATIONINDEX -= 1;
      else ANIMATIONINDEX = ANIMATIONINDEX += 1;
    }
    // ANIMATIONINDEX is out of the limit
    else
    {
      // Repeat to do?
      if (ISREPEAT && (REPEATCOUNTER > 0))
      {
        // Restart the animation
        ANIMATIONS.RestartAnimation(param.index);

        // Initialisation
        REPEATCOUNTER -= 1;

        // Index
        if (ISINVERT) ANIMATIONINDEX = ANIMATIONINDEXSTOP;
        else ANIMATIONINDEX = ANIMATIONINDEXSTART;
      }
      // Bounce to do?
      else if (ISBOUNCE && (REPEATCOUNTER > 0))
      {
        // Restart the animation
        ANIMATIONS.RestartAnimation(param.index);

        // Initialisation
        REPEATCOUNTER -= 1;
        ISINVERT = !ISINVERT; //invert the invert (following ??)

        // Index
        if (ISINVERT) ANIMATIONINDEX = ANIMATIONINDEXSTOP;
        else ANIMATIONINDEX = ANIMATIONINDEXSTART;
      }
      // Nothing more to do
      else
      {
        // Stop the animation
        ANIMATIONS.StopAnimation(param.index);

        // Blank the strip if needed
        if (ISENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));
      }
    }
  }
}
