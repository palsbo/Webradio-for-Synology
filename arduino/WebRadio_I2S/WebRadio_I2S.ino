/*
  The following Audio library can be found here: https://github.com/earlephilhower/ESP8266Audio/blob/master/src/AudioFileSourceICYStream.h
  Thanks to the author.
*/
#ifdef ESP8266
#include <ESP8266WiFi.h>
const int preallocateBufferSize = 5 * 1024;
const int preallocateCodecSize = 29192; // MP3 codec max mem needed
#else
#include <WiFi.h>
const int preallocateBufferSize = 26 * 1024;
const int preallocateCodecSize = 85332; // AAC+SBR codec max mem needed
#endif
#include "sock.h"
#define DAC   //  uncomment if I2S DAC module is used
#define AUDIOGENERATOR  AudioGeneratorMP3
#define AUDIOSOURCE AudioFileSourceHTTPStream

#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorAAC.h"
#ifdef DAC
#define AUDIOOUTPUT AudioOutputI2S
#include "AudioOutputI2S.h"
#else
#define AUDIOOUTPUT AudioOutputI2SNoDAC
#include "AudioOutputI2SNoDAC.h"
/*
   if I2S DAC module is not used, use  #include "AudioOutputI2SNoDAC.h" to get sound from pin RX on ESP8266
*/
#endif


int volume = 50;
bool newUrl = false;
int retryms = 0;

void *preallocateBuffer = NULL;
void *preallocateCodec = NULL;

/*  Configuration values to be filled in as needed  */
#define SSID  "..";
#define PASS  "..";
#define WSHOST  "..";
#define WSPORT  81;
#define MQTT_CLIENT ESP.getChipId()
#define MQTT_SERVER "...";
#define MQTT_PORT 17332;
#define MQTT_USER "...";
#define MQTT_PASS "..";

#include <init.h> //  I use this library file to redefine the configuration values - remove if config data are defined abowe

const char *ssid = SSID;
const char *pass = PASS;
const char *wshost = WSHOST;
int wsport = WSPORT;

AUDIOGENERATOR *mp3;
AUDIOSOURCE *file;
AudioFileSourceBuffer *buff;
#ifdef DAC
AudioOutputI2S *out;
#else
AudioOutputI2SNoDAC *out;
#endif

SOCK sock;

const char *URL = "http://streaming.shoutcast.com/80sPlanet?lang=en-US";
bool streamstatus = false;
uint32_t startdelay;

char * ptr;

static void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
  if (string[0] != '\0') {
    Serial.printf("%s %s\n", type, string);
    Serial.flush();
  }
}

void StatusCallback(void *cbData, int code, const char *string) {
  const char *ptr = reinterpret_cast<const char *>(cbData);
  // Note that the string may be in PROGMEM, so copy it to RAM for printf
  char s1[64];
  strncpy_P(s1, string, sizeof(s1));
  s1[sizeof(s1) - 1] = 0;
  Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
  Serial.flush();
}

void stopPlay() {
  if (mp3) {
    if (mp3->isRunning()) {
      mp3->stop();
      delete mp3;
      mp3 = NULL;
    }
  }
  if (buff) {
    buff->close();
    delete buff;
    buff = NULL;
  }
  if (file) {
    file->close();
    delete file;
    file = NULL;
  }
  delay(1000);
}

void startPlay() {
  file = new AudioFileSourceICYStream(URL);
  file->RegisterMetadataCB(MDCallback, (void*)"ICY");
  buff = new AudioFileSourceBuffer(file, preallocateBuffer, preallocateBufferSize);
  //buff->RegisterStatusCB(StatusCallback, NULL);
  mp3 = new AudioGeneratorMP3(preallocateCodec, preallocateCodecSize);
  //mp3->RegisterStatusCB(StatusCallback, NULL);
  Serial.printf_P("Decoder start...\n");
  mp3->begin(buff, out);
  out->SetGain(((float)volume) / 100.0);
  newUrl = false;
}

void handleUrl(char * newURL) {
  URL = newURL;
  stopPlay();
  startPlay();
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Connecting to WiFi");
  WiFi.disconnect();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nConnected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  //  Prepare player
  preallocateBuffer = malloc(preallocateBufferSize);
  //preallocateCodec = malloc(preallocateCodecSize);
  file = NULL;
  buff = NULL;
  out = new AudioOutputI2S();
  mp3 = NULL;
  //  Websocket events
  sock.on("connect", [](char* data) {
    Serial.printf("[WSc] Connected! %s\n", data);
  });
  sock.on("disconnect", [](char* data) {
    Serial.printf("[WSc] Disconnected!%s\n", data);
  });
  sock.on("url", [](char* data) {
    Serial.printf("[WSc] URL! '%s'\n", data);
    handleUrl(data);
  });
  /*
    sock.on("StreamTitle", [](char* data) {
    Serial.printf("[WSc] Title! '%s'\n", data);
    });
  */
  sock.begin(wshost, wsport);
}

void loop() {
  if (mp3 && mp3->isRunning()) {
    if (!mp3->loop()) {
      stopPlay();
      newUrl = true;
      retryms = millis() + 2000;
    }
  }
  if ((millis() > retryms) && newUrl) startPlay();
  sock.loop();
}

