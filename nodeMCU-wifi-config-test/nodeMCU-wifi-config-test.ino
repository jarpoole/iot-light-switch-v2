
#include <IotWebConf.h>
#include <FS.h>

// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char thingName[] = "jSwitch";

// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char wifiInitialApPassword[] = "password";

#define STRING_LEN 128
#define NUMBER_LEN 32

// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "v1.7"

// -- When CONFIG_PIN is pulled to ground on startup, the Thing will use the initial
//      password to buld an AP. (E.g. in case of lost password)
#define CONFIG_PIN 8

// -- Status indicator pin.
//      First it will light up (kept LOW), on Wifi connection it will blink,
//      when connected to the Wifi it will turn off (kept HIGH).
#define STATUS_PIN LED_BUILTIN

// -- Callback method declarations.
void configSaved();
boolean formValidator();

DNSServer dnsServer;
WebServer server(80);

char stringParamValue[STRING_LEN];
char intParamValue[NUMBER_LEN];
char floatParamValue[NUMBER_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);

IotWebConfSeparator separator1 = IotWebConfSeparator("User Settings");
IotWebConfParameter userIDParam = IotWebConfParameter("User ID", "stringParam", stringParamValue, STRING_LEN, "text", "e.g. jaredpoole");

IotWebConfSeparator separator2 = IotWebConfSeparator("Device Settings");
IotWebConfParameter pollingRateParam = IotWebConfParameter("Polling rate", "intParam", intParamValue, NUMBER_LEN, "number", "0-8sec", NULL, "min='0' max='8' step='1'");

//IotWebConfParameter floatParam = IotWebConfParameter("Float param", "floatParam", floatParamValue, NUMBER_LEN, "number", "e.g. 23.4", NULL, "step='0.1'");

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //iotWebConf.setStatusPin(STATUS_PIN);
  //iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.addParameter(&separator1);
  iotWebConf.addParameter(&userIDParam);
  iotWebConf.addParameter(&separator2);
  iotWebConf.addParameter(&pollingRateParam);
  //iotWebConf.addParameter(&floatParam);
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.getApTimeoutParameter()->visible = true;

  // -- Initializing the configuration.
  iotWebConf.init();

  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ handleFileRead(); });
  //server.onNotFound([](){ iotWebConf.handleNotFound(); });
  
  Serial.println("Ready.");
}

void loop()
{
  // -- doLoop should be called as frequently as possible.
  iotWebConf.doLoop();
}


// Handle web requests to "/" path.
void handleRoot(){
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal()){
    return;// -- Captive portal request were already served.
  }
  File file = SPIFFS.open("/root.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead() { // send the right file to the client (if it exists)
  if (iotWebConf.handleCaptivePortal()){
    return true;// -- Captive portal request were already served.
  }
  String path = server.uri();
  Serial.println("handleFileRead: " + path);
  String contentType;
  if (path.endsWith("/") || !SPIFFS.exists(path) ){
    path = "/404.html";         // If a folder is requested, send the 404 file
    contentType = "text/html";
    Serial.println("\tFile Not Found");
  }else{
    contentType = getContentType(path);            // Get the MIME type
  }
  File file = SPIFFS.open(path, "r");                 // Open it
  size_t sent = server.streamFile(file, contentType); // And send it to the client
  file.close();                                       // Then close the file again
  return true;
}

void configSaved(){
  Serial.println("Configuration was updated.");
}

boolean formValidator()
{
  Serial.println("Validating form.");
  boolean valid = true;

  int l = server.arg(userIDParam.getId()).length();
  if (l < 1){
    userIDParam.errorMessage = "User ID cannot be empty";
    valid = false;
  }

  return valid;
}
