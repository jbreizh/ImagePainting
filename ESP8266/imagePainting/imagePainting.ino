//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
//#define STA       // Decomment this to use STA mode instead of AP
//#define DNS       // Decomment this to use DNS
//#define DEBUGER     // Decomment this to debug the code
#define BUTTON      // Decomment this to use BUTTON
#define FEATURE DotStarBgrFeature // Neopixels : NeoGrbFeature / Dotstars : DotStarBgrFeature
#define METHOD DotStarSpiMethod // Neopixels :Neo800KbpsMethod / Dotstars : DotStarSpiMethod
//Dotstars : DATA_PIN : MOSI / CLOCK_PIN :SCK (Wemos D1 mini DATA_PIN=D7(GREEN) CLOCK_PIN=D5 (Yellow))
//Neopixels : DATA_PIN : RDX0/GPIO3 (Wemos D1 mini DATA_PIN=RX)
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <LittleFS.h>
#ifdef DNS
#include <DNSServer.h>
#endif

// LED --------------
const int NUMPIXELS = 119;
NeoPixelBus<FEATURE, METHOD> STRIP(NUMPIXELS);
// end LED -----------

// WEBSERVER --------------
ESP8266WebServer server;
// end WEBSERVER -----------

// WIFI --------------
#ifdef STA // STA Mode
const char* ssid = "Moto C Plus 1105"; // your wifi ssid for STA mode
const char* password = "12345678"; // your wifi password for AP mode
#else // AP Mode
const char* ssid = "imagePainting"; // wifi ssid for AP mode
IPAddress apIP(192, 168, 1, 1); // wifi IP for AP mode
#endif
// end WIFI -----------

// DNS --------------
#ifdef DNS
DNSServer dnsServer;
const byte DNS_PORT = 53;
#endif
// end DNS -----------

// FS --------------
fs::File UPLOADFILE; // hold uploaded file
const char *CONFIGPATH = "config.json";  // config file
String BMPPATH = "welcome.bmp";
NeoBitmapFile<FEATURE, fs::File> NEOBMPFILE;
// end FS -----------

// ANIMATION --------------
NeoPixelAnimator ANIMATIONS(1); // NeoPixel animation management object
uint16_t INDEXMIN; //Min index possible
uint16_t INDEXSTART; //Min index chosen
uint16_t INDEX; // Current index
uint16_t INDEXSTOP; //Max index chosen
uint16_t INDEXMAX; //Max index possible
// end ANIMATION --------------

// RUNTIME --------------
long COUNTDOWN = 0; long COUNTDOWNCOUNTER;
bool ISCOUNTDOWN = false;
uint8_t DELAY = 15;
uint8_t BRIGHTNESS = 25;
uint8_t REPEAT = 1; uint8_t REPEATCOUNTER;
bool ISINVERT = false;
bool ISREPEAT = false;
bool ISBOUNCE = false;
uint8_t PAUSE = 1; uint8_t PAUSECOUNTER;
bool ISPAUSE = false;
bool ISCUT = false;
HtmlColor COLOR = HtmlColor(0xffffff);
bool ISENDOFF = false;
bool ISENDCOLOR = false;
// end RUNTIME --------------

// BUTTON --------------
#ifdef BUTTON
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
#endif
// end BUTTON-----------

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
      // it does assume each element is only 8bits, but this currently is the case
      // This could be replaced with a LinearBlend for safty but is less optimized
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

typedef BrightnessShader<FEATURE::ColorObject> BrightShader;

BrightShader SHADER;
//end SHADER --------------

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void setup()
{
  // FS setup
  LittleFS.begin();

  // Serial setup
  Serial.begin(115200);

  // Wifi setup
#ifdef STA //STA Mode
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
#else //AP Mode
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
#endif

  // DNS setup
#ifdef DNS
  dnsServer.start(DNS_PORT, "*", apIP);
#endif

  // Webserver setup
  // list available files
  server.on("/list", HTTP_GET, handleFileList);

  // delete file
  server.on("/delete", HTTP_DELETE, handleFileDelete);

  // handle file upload
  server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "UPLOAD SUCCESS");
  }, handleFileUpload);

  // handle Play
  server.on("/play", HTTP_GET, handlePlay);

  // handle Stop
  server.on("/stop", HTTP_GET, handleStop);

  // handle light
  server.on("/light", HTTP_GET, handleLight);

  // handle burn
  server.on("/burn", HTTP_GET, handleBurn);

  // handle parameter Read
  server.on("/parameterRead", HTTP_GET, handleParameterRead);

  // handle parameter Save
  server.on("/parameterSave", HTTP_GET, handleParameterSave);

  // handle parameter Write
  server.on("/parameterWrite", HTTP_POST, handleParameterWrite);

  // handle parameter Restore
  server.on("/parameterRestore", HTTP_GET, handleParameterRestore);

  // handle system Read
  server.on("/systemRead", HTTP_GET, handleSystemRead);

  // called when the url is not defined
  server.onNotFound([]() {
    handleFileRead(server.uri());
  });

  // Webserver start
  server.begin();

  // LED setup
  STRIP.Begin();
  STRIP.ClearTo(RgbColor(0, 0, 0));
  SHADER.setBrightness(BRIGHTNESS);
  bitmapLoad(BMPPATH);

  // Button setup
#ifdef BUTTON
  pinMode(BTNA_PIN, INPUT_PULLUP);
  pinMode(BTNB_PIN, INPUT_PULLUP);
#endif
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void loop()
{
  // To handle the DNS
#ifdef DNS
  dnsServer.processNextRequest();
#endif

  // To handle the webserver
  server.handleClient();

  // To handle the LED animation
  ANIMATIONS.UpdateAnimations();
  STRIP.Show();

  // To handle the buttons
#ifdef BUTTON
  // To handle the button A
  if (digitalRead(BTNA_PIN) == LOW)
  {
    if (!ISBTNA)
    {
      ISBTNA = true;
      BTNATIMER = millis();
    }
    if ((millis() - BTNATIMER > HOLDTIME) && (!ISBTNAHOLD))
    {
      ISBTNAHOLD = true;
      burn();
    }
  }
  else if (ISBTNA)
  {
    if ((millis() - BTNATIMER > DEBOUNCETIME) && (!ISBTNAHOLD))
    {
      play();
    }
    ISBTNA = false;
    ISBTNAHOLD = false;
  }

  // To handle the button B
  if (digitalRead(BTNB_PIN) == LOW)
  {
    if (!ISBTNB)
    {
      ISBTNB = true;
      BTNBTIMER = millis();
    }
    if ((millis() - BTNBTIMER > HOLDTIME) && (!ISBTNBHOLD))
    {
      ISBTNBHOLD = true;
      light();
    }
  }
  else if (ISBTNB)
  {
    if ((millis() - BTNBTIMER > DEBOUNCETIME) && (!ISBTNBHOLD))
    {
      stopB();
    }
    ISBTNB = false;
    ISBTNBHOLD = false;
  }
#endif
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
String getContentType(String filename)
{
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".bmp")) return "image/bmp";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".css")) return "text/css";
  return "text/plain";
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
String systemRead()
{
  // New json document
  StaticJsonDocument<500> jsonDoc;

  // Store system parameter in json document
  FSInfo fs_info;
  LittleFS.info(fs_info);
  jsonDoc["usedBytes"] = fs_info.usedBytes;
  jsonDoc["totalBytes"] = fs_info.totalBytes;
  jsonDoc["numPixels"] = NUMPIXELS;

  // Convert json document to String and return it
  String systemParameter = "";
  serializeJson(jsonDoc, systemParameter);
  return systemParameter;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleSystemRead()
{
  // Read system parameter
  String systemParameter = systemRead();

  // System parameter are read
  server.send(200, "application/json", systemParameter);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
String parameterRead()
{
  // New json document
  StaticJsonDocument<500> jsonDoc;

  // Store parameter in json document
  jsonDoc["delay"] = DELAY;
  jsonDoc["brightness"] = BRIGHTNESS;
  //
  jsonDoc["countdown"] = COUNTDOWN;
  jsonDoc["iscountdown"] = ISCOUNTDOWN;
  //
  jsonDoc["repeat"] = REPEAT;
  jsonDoc["isinvert"] = ISINVERT;
  jsonDoc["isrepeat"] = ISREPEAT;
  jsonDoc["isbounce"] = ISBOUNCE;
  //
  jsonDoc["pause"] = PAUSE;
  jsonDoc["ispause"] = ISPAUSE;
  jsonDoc["iscut"] = ISCUT;
  //
  char color[9];
  COLOR.ToNumericalString(color, 9);
  jsonDoc["color"] = color;
  jsonDoc["isendoff"] = ISENDOFF;
  jsonDoc["isendcolor"] = ISENDCOLOR;
  //
  jsonDoc["indexMin"] = INDEXMIN;
  jsonDoc["indexStart"] = INDEXSTART;
  jsonDoc["indexStop"] = INDEXSTOP;
  jsonDoc["indexMax"] = INDEXMAX;
  jsonDoc["bmpPath"] = BMPPATH;

  // convert json document to String and return it
  String parameter = "";
  serializeJson(jsonDoc, parameter);
  return parameter;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterRead()
{
  // read the parameter
  String parameter = parameterRead();

  // Parameter are read
  server.send(200, "application/json", parameter);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterSave()
{
  // read the parameter
  String parameter = parameterRead();

  // Create or open CONFIGPATH
  File configFile = LittleFS.open(CONFIGPATH, "w");

  // Save parameter in configFile
  configFile.print(parameter);

  // Close configFile
  configFile.close();

  // Parameter are save
  server.send(200, "text/html",  "SAVE SUCCESS");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void parameterWrite(String stringParameter)
{
  // New json document
  StaticJsonDocument<500> jsonDoc;

  // Convert json String to json object
  DeserializationError error = deserializeJson(jsonDoc, stringParameter);

  // Json not right ?
  if (error)
  {
    // Error wrong json
    return server.send(500, "text/html", "WRITE ERROR : WRONG JSON");
  }

  // Running or paused animation ?
  if (ANIMATIONS.IsAnimationActive(0) || ANIMATIONS.IsPaused())
  {
    // Error not available
    return server.send(403, "text/html", "WRITE ERROR : NOT AVAILABLE");
  }

  // Write parameter in the ESP8266
  if (!jsonDoc["delay"].isNull()) DELAY = jsonDoc["delay"];
  if (!jsonDoc["brightness"].isNull())
  {
    BRIGHTNESS = jsonDoc["brightness"];
    SHADER.setBrightness(BRIGHTNESS);
  }
  //
  if (!jsonDoc["countdown"].isNull()) COUNTDOWN = jsonDoc["countdown"];
  if (!jsonDoc["iscountdown"].isNull())ISCOUNTDOWN = jsonDoc["iscountdown"];
  //
  if (!jsonDoc["repeat"].isNull()) REPEAT = jsonDoc["repeat"];
  if (!jsonDoc["isinvert"].isNull())ISINVERT = jsonDoc["isinvert"];
  if (!jsonDoc["isrepeat"].isNull())ISREPEAT = jsonDoc["isrepeat"];
  if (!jsonDoc["isbounce"].isNull())ISBOUNCE = jsonDoc["isbounce"];
  //
  if (!jsonDoc["pause"].isNull()) PAUSE = jsonDoc["pause"];
  if (!jsonDoc["ispause"].isNull())ISPAUSE = jsonDoc["ispause"];
  if (!jsonDoc["iscut"].isNull())ISCUT = jsonDoc["iscut"];
  //
  if (!jsonDoc["color"].isNull())COLOR.Parse<HtmlShortColorNames>(jsonDoc["color"].as<String>());
  if (!jsonDoc["isendoff"].isNull())ISENDOFF = jsonDoc["isendoff"];
  if (!jsonDoc["isendcolor"].isNull())ISENDCOLOR = jsonDoc["isendcolor"];
  //
  if (!jsonDoc["indexStart"].isNull())INDEXSTART = jsonDoc["indexStart"];
  if (!jsonDoc["indexStop"].isNull())INDEXSTOP = jsonDoc["indexStop"];
  if (!jsonDoc["bmpPath"].isNull() && jsonDoc["bmpPath"].as<String>() != BMPPATH)
  {
    //  Args ; path ; type ; bitmap not right ?
    if ((!LittleFS.exists(jsonDoc["bmpPath"].as<String>())) || (getContentType(jsonDoc["bmpPath"].as<String>()) != "image/bmp") || (!bitmapLoad(jsonDoc["bmpPath"].as<String>())))
    {
      // Load welcome.bmp
      bitmapLoad("welcome.bmp");
      // Error bitmap
      return server.send(500, "text/html", "WRITE ERROR : WRONG BITMAP");
    }
  }

  // Parameter are write
  server.send(200, "text/html",  "WRITE SUCCESS");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool bitmapLoad(String path)
{
  // Open requested file on LittleFS
  fs::File bmpFile = LittleFS.open(path, "r");

  // Check and initialize NEOBMPFILE from the BMPFILE
  bool success = NEOBMPFILE.Begin(bmpFile);

  // Update BMPPATH
  BMPPATH = path;

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
void handleParameterWrite()
{
  // Write parameter
  parameterWrite(server.arg("plain"));
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleParameterRestore()
{
  // Open CONFIGPATH
  File configFile = LittleFS.open(CONFIGPATH, "r");

  // read configFile
  String configString;
  while (configFile.available())
  {
    configString += char(configFile.read());
  }

  // Close configFile
  configFile.close();

  // Write parameter
  parameterWrite(configString);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileDelete()
{
  // parse file from request
  String path = server.arg("plain");

  // protect system files
  if ( path == "" || path == "/" || path == "config.json" || path == "index.html" || path == "welcome.bmp" || path == "title.png") return server.send(500, "text/plain", "DELETE ERROR : SYSTEM FILE");

  // check if the file exists
  if (!LittleFS.exists(path)) return server.send(404, "text/plain", "DELETE ERROR : FILE NOT FOUND");

  // if delete current bitmap reload defaut bitmap
  if (path == BMPPATH)  bitmapLoad("welcome.bmp");

  // Delete the file
  LittleFS.remove(path);

  // File is delete
  server.send(200, "text/plain", "DELETE SUCCESS");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileRead(String path)
{
  // Serve index file when top root path is accessed
  if (path.endsWith("/")) path += "index.html";

  // Check if the file exists
#ifdef DNS
  if (!LittleFS.exists(path))
  {
    const char *metaRefreshStr = "<head><meta http-equiv=\"refresh\" content=\"0; url=http://192.168.1.1/index.html\" /></head><body><p>redirecting...</p></body>";
    return server.send(200, "text/html", metaRefreshStr);
  }
#else
  if (!LittleFS.exists(path)) return server.send(404, "text/plain", "READ ERROR : FILE NOT FOUND");
#endif
  // Open the file
  fs::File file = LittleFS.open(path, "r");

  // Display the file on the client's browser
  server.streamFile(file, getContentType(path));

  // Close the file
  file.close();
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

    //check if the file already exist
    //if (LittleFS.exists(filename))
    //{
    //  return server.send(500, "text/plain", "UPLOAD ERROR : FILE ALREADY EXIST");
    //}

    // Open the file for writing in LittleFS (create if it doesn't exist)
    UPLOADFILE = LittleFS.open(filename, "w");
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
    //Close the file
    if (UPLOADFILE)
      UPLOADFILE.close();
  }
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleFileList()
{
  // Assuming there are no subdirectories
  fs::Dir dir = LittleFS.openDir("/");

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
    String name = String(entry.name());

    // Write the entry in the list (Hide system file)
    if (!(name == "index.html" || name == "title.png" || name == "config.json")) fileList.add(name);

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
  String htmlMsg = play();
  server.send(200, "text/plain", htmlMsg);
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
String play()
{
  // Html msg
  String htmlMsg = "";

  // Animation is paused?
  if (ANIMATIONS.IsPaused())
  {
    // Resume animation
    ANIMATIONS.Resume();

    // Animation is resume
    htmlMsg = "RESUME";
  }
  // Animation is active?
  else if (ANIMATIONS.IsAnimationActive(0))
  {
    // Pause animation
    ANIMATIONS.Pause();

    // Blank the strip if needed
    if (ISENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));
    if (ISENDCOLOR) STRIP.ClearTo(SHADER.Apply(0, COLOR));

    // Animation is paused
    htmlMsg = "PAUSE";
  }
  // No animation !!! let s start a new one
  else
  {
    // Index
    if (ISINVERT) INDEX = INDEXSTOP;
    else INDEX = INDEXSTART;

    // Countdown
    COUNTDOWNCOUNTER = millis();

    // Repeat
    REPEATCOUNTER = REPEAT;

    // Pause
    PAUSECOUNTER = 2 * PAUSE;

    // Launch a new animation
    ANIMATIONS.StartAnimation(0, DELAY, updateAnimation);

    // New animation is launch
    htmlMsg = "PLAY";
  }
  return htmlMsg;
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleStop()
{
  stopB();
  server.send(200, "text/plain", "STOP");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void stopB()
{
  // Stop animation
  ANIMATIONS.StopAnimation(0);
  ANIMATIONS.Resume(); // remove the pause flag to stop paused animation

  // Turn off the strip
  STRIP.ClearTo(RgbColor(0, 0, 0));
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleLight()
{
  light();
  server.send(200, "text/plain", "LIGHT");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void light()
{
  // Stop animation
  ANIMATIONS.StopAnimation(0);
  ANIMATIONS.Resume(); // remove the pause flag to stop paused animation

  //turn on the strip
  STRIP.ClearTo(SHADER.Apply(0, COLOR));
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void handleBurn()
{
  burn();
  server.send(200, "text/plain", "BURN");
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void burn()
{
  // Stop animation
  ANIMATIONS.StopAnimation(0);
  ANIMATIONS.Resume(); // remove the pause flag to stop paused animation

  //turn on the strip
  NEOBMPFILE.Render<BrightShader>(STRIP, SHADER, 0, 0, INDEXSTART, NEOBMPFILE.Width());
}

//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
void updateAnimation(const AnimationParam & param)
{
  // Wait for this animation to complete,
  if (param.state == AnimationState_Completed)
  {
    // INDEX is in the limit
    if ((INDEXSTART <= INDEX) && (INDEX <= INDEXSTOP))
    {
      // Restart the animation
      ANIMATIONS.RestartAnimation(param.index);

      // Countdown to do?
      if (ISCOUNTDOWN && (millis() - COUNTDOWNCOUNTER <= COUNTDOWN))
      {
        // Wait
      }
      // Pause to do?
      else if ((ISPAUSE || ISCUT) && (PAUSECOUNTER <= PAUSE))
      {
        //Initialisation
        if (PAUSECOUNTER > 1) PAUSECOUNTER -= 1;
        else PAUSECOUNTER = 2 * PAUSE;

        // Blank or color the strip if needed
        if (ISENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));
        if (ISENDCOLOR) STRIP.ClearTo(SHADER.Apply(0, COLOR));

        // Cut the bitmap is needed
        if (ISCUT)
        {
          // Index
          if (ISINVERT) INDEX -= 1;
          else INDEX += 1;
        }
      }
      // No countdown/pause to do !!! so let's play
      else
      {
        // Fil the strip : bitmap is crop to fit the strip !!!
        NEOBMPFILE.Render<BrightShader>(STRIP, SHADER, 0, 0, INDEX, NEOBMPFILE.Width());

        // Index
        if (ISINVERT) INDEX -= 1;
        else INDEX += 1;

        //
        if (ISPAUSE || ISCUT) PAUSECOUNTER -= 1;
      }
    }
    // INDEX is out of the limit
    else
    {
      // Repeat to do?
      if ((ISREPEAT || ISBOUNCE) && (REPEATCOUNTER > 0))
      {
        // Restart the animation
        ANIMATIONS.RestartAnimation(param.index);

        // Initialisation
        REPEATCOUNTER -= 1;
        if (ISBOUNCE) ISINVERT = !ISINVERT; //invert the invert (following ??)

        // Index
        if (ISINVERT) INDEX = INDEXSTOP;
        else INDEX = INDEXSTART;
      }
      // Nothing more to do
      else
      {
        // Stop the animation
        ANIMATIONS.StopAnimation(param.index);

        // Blank or color the strip if needed
        if (ISENDOFF) STRIP.ClearTo(RgbColor(0, 0, 0));
        if (ISENDCOLOR) STRIP.ClearTo(SHADER.Apply(0, COLOR));

#ifdef DEBUGER
        Serial.print("complete time :");
        Serial.println(millis() - COUNTDOWNCOUNTER);
#endif
      }
    }
  }
}
