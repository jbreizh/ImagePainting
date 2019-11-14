#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
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
fs::File UPLOADFILE; // hold uploaded file
// end FS -----------

// BITMAP --------------
String BMPPATH = "/welcome.bmp";
fs::File BMPFILE; // hold bitmap file
NeoBitmapFile<DotStarBgrFeature, fs::File> NEOBMPFILE;
// end BITMAP -----------

// ANIMATION --------------
NeoPixelAnimator ANIMATIONS(1); // NeoPixel animation management object
uint16_t INDEXMIN; //Min index possible
uint16_t INDEXSTART; //Min index chosen
uint16_t INDEX; // Current index
uint16_t INDEXSTOP; //Max index chosen
uint16_t INDEXMAX; //Max index possible
// end ANIMATION --------------

// RUNTIME --------------
uint8_t DELAY = 30;
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

  // handle bitmap Play
  server.on("/play", HTTP_GET, handlePlay);

  // handle bitmap Stop
  server.on("/stop", HTTP_GET, handleStop);

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
  bitmapLoad(BMPPATH);
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
  jsonDoc["indexMin"] = INDEXMIN;
  jsonDoc["indexStart"] = INDEXSTART;
  jsonDoc["indexStop"] = INDEXSTOP;
  jsonDoc["indexMax"] = INDEXMAX;
  jsonDoc["bmpPath"] = BMPPATH;

  // convert json document to String
  String msg = "";
  serializeJson(jsonDoc, msg);

  // Parameter are read
  server.send(200, "application/json", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterWrite()
{
  // Html code and msg
  uint16_t htmlCode;
  String htmlMsg = "";

  // New json document
  StaticJsonDocument<300> jsonDoc;

  // Convert json String to json object
  DeserializationError error = deserializeJson(jsonDoc, server.arg("plain"));

  // Json not right ?
  if (error)
  {
    // Html code and msg AKA do nothing
    htmlCode = 500; // Internal Error
    htmlMsg = "WRITE ERROR : WRONG JSON -> ";
    htmlMsg += error.c_str();
  }
  // Running or paused animation ?
  else if (ANIMATIONS.IsAnimationActive(0) || ANIMATIONS.IsPaused())
  {
    // Html code and msg AKA do nothing
    htmlCode = 403; // Forbidden
    htmlMsg = "WRITE ERROR : NOT AVAILABLE";
  }
  // Changing bitmap ?
  else if (jsonDoc["bmpPath"].as<String>() != BMPPATH)
  {
    //  Args ; path ; type ; bitmap not right ?
    if ((jsonDoc["bmpPath"].as<String>() == "") || (!SPIFFS.exists(jsonDoc["bmpPath"].as<String>())) || (getContentType(jsonDoc["bmpPath"].as<String>()) != "image/bmp") || (!bitmapLoad(jsonDoc["bmpPath"].as<String>())))
    {
      // Load /error.bmp
      BMPPATH = "/error.bmp";
      bitmapLoad(BMPPATH);

      // Html code and msg
      htmlCode = 500; // Internal Error
      htmlMsg = "WRITE ERROR : BAD ARGS OR FILE OR BITMAP";
    }
    // No problem ?
    else
    {
      // Load the new bitmap
      BMPPATH = jsonDoc["bmpPath"].as<String>();

      // Html code and msg
      htmlCode = 200; // OK
      htmlMsg = "WRITE SUCCESS : BITMAP LOAD";
    }
  }
  // No problem ?
  else
  {
    // Write parameters in the ESP8266
    DELAY = jsonDoc["delay"];
    BRIGHTNESS = jsonDoc["brightness"];
    SHADER.setBrightness(BRIGHTNESS);
    REPEAT = jsonDoc["repeat"];
    ISREPEAT = jsonDoc["isrepeat"];
    ISBOUNCE = jsonDoc["isbounce"];
    ISINVERT = jsonDoc["isinvert"];
    ISENDOFF = jsonDoc["isendoff"];
    INDEXSTART = jsonDoc["indexStart"];
    INDEXSTOP = jsonDoc["indexStop"];
    
    // Html code and msg
    htmlCode = 200; // OK
    htmlMsg = "WRITE SUCCESS : PARAMETERS SET";
  }

  // Parameter are write
  server.send(htmlCode, "text/html", htmlMsg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool bitmapLoad(String path)
{
  // Close the old bitmap
  BMPFILE.close();
  NEOBMPFILE.Begin(BMPFILE);

  // Open requested file on SPIFFS
  BMPFILE = SPIFFS.open(path, "r");

  // Check and initialize NEOBMPFILE from the BMPFILE
  bool success = NEOBMPFILE.Begin(BMPFILE);

  // Update the index possible
  INDEXMIN = 0;
  INDEXMAX = NEOBMPFILE.Height() - 1;

  // Update the index chosen
  INDEXSTART = INDEXMIN;
  INDEXSTOP = INDEXMAX;

  // Bitmap is loaded
  return success;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileDelete()
{
  // parse parameter from request
  String path = server.arg("file");

  // make sure we get a file name as a URL argument and protect root path
  if (path == "" || path == "/") return server.send(500, "text/plain", "DELETE ERROR : BAD ARGS");

  // protect system files
  if (path == "/error.bmp" || path == "/welcome.bmp") return server.send(500, "text/plain", "DELETE ERROR : SYSTEM FILE");

  // check if the file exists
  if (!SPIFFS.exists(path)) return server.send(404, "text/plain", "DELETE ERROR : FILE NOT FOUND!");

  // if delete current bitmap reload defaut bitmap
  if (path == BMPPATH)
  {
    BMPPATH = "/welcome.bmp";
    bitmapLoad(BMPPATH);
  }

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
    UPLOADFILE = SPIFFS.open(filename, "w");
  }

  // Upload in progress
  else if (upload.status == UPLOAD_FILE_WRITE)
  {
    //Write the received bytes to the file
    if (UPLOADFILE) UPLOADFILE.write(upload.buf, upload.currentSize);
  }

  // Upload end
  else if (upload.status == UPLOAD_FILE_END)
  {
    if (UPLOADFILE)
      UPLOADFILE.close();
  }
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileList()
{
  // Assuming there are no subdirectories
  fs::Dir dir = SPIFFS.openDir("/");

  // New json document
  StaticJsonDocument<1000> jsonDoc;

  // Store parameter in json document
  JsonArray fileList = jsonDoc.createNestedArray("fileList");

  // Scan the files
  while (dir.next())
  {
    // Open the entry
    fs::File entry = dir.openFile("r");

    // Get the name
    String name = String(entry.name());//.substring(1);

    // Write the entry in the list (Hide system file)
    if (name != "/index.html")  fileList.add(name);

    // Close the entry
    entry.close();
  }

  // convert json document to String
  String msg = "";
  serializeJson(jsonDoc, msg);

  // Parameter are read
  server.send(200, "application/json", msg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handlePlay()
{
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
    if (ISINVERT) INDEX = INDEXSTOP;
    else INDEX = INDEXSTART;

    // Repeat
    REPEATCOUNTER = REPEAT;

    // Launch a new animation
    ANIMATIONS.StartAnimation(0, DELAY, updateAnimation);

    // New animation is launch
    server.send(200, "text/plain", "PLAY");
  }
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleStop()
{
  // Stop animation
  ANIMATIONS.StopAnimation(0);
  ANIMATIONS.Resume();

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
    // INDEX is in the limit
    if ((INDEXSTART <= INDEX) && (INDEX <= INDEXSTOP))
    {
      // Restart the animation
      ANIMATIONS.RestartAnimation(param.index);

      // Fil the strip : bitmap is crop to fit the strip !!!
      NEOBMPFILE.Render<BrightShader>(STRIP, SHADER, 0, 0, INDEX, NEOBMPFILE.Width());

      // Index
      if (ISINVERT) INDEX -= 1;
      else INDEX += 1;
    }
    // INDEX is out of the limit
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
        if (ISINVERT) INDEX = INDEXSTOP;
        else INDEX = INDEXSTART;
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
        if (ISINVERT) INDEX = INDEXSTOP;
        else INDEX = INDEXSTART;
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
