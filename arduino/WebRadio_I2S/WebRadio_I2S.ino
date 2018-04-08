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

#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioGeneratorAAC.h"
#include "AudioOutputI2S.h"
#include "AudioOutputI2SNoDAC.h"

#define AUDIOOUTPUT AudioOutputI2S
#define AUDIOGENERATOR  AudioGeneratorMP3
#define AUDIOSOURCE AudioFileSourceHTTPStream
#define AUDIOBUFFER AudioFileSourceBuffer

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

int   volume = 100;
bool  newUrl = false;
int   retryms = 0;
void  *preallocateBuffer = NULL;
void  *preallocateCodec = NULL;

const char *ssid = SSID;
const char *pass = PASS;
const char *wshost = WSHOST;
int wsport = WSPORT;

const char *URL;

AUDIOGENERATOR *decoder;
AUDIOSOURCE *file;
AUDIOBUFFER *buff;
AUDIOOUTPUT *out;
SOCK sock;

void stopPlay() {
  if (decoder) {
    if (decoder->isRunning()) {
      decoder->stop();
      delete decoder;
      decoder = NULL;
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
}

void startPlay() {
  file = new AUDIOSOURCE(URL);
  buff = new AUDIOBUFFER(file, preallocateBuffer, preallocateBufferSize);
  decoder = new AUDIOGENERATOR(preallocateCodec, preallocateCodecSize);
  Serial.printf_P("Decoder start...\n");
  decoder->begin(buff, out);
  out->SetGain(((float)volume) / 100.0);
  newUrl = false;
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
  preallocateCodec = malloc(preallocateCodecSize);
  file = NULL;
  buff = NULL;
  out = new AUDIOOUTPUT();
  decoder = NULL;

  //  Websocket events

  sock.on("connect", [](char* data) {
    Serial.printf("[WSc] Connected! %s\n", data);
  });
  sock.on("disconnect", [](char* data) {
    Serial.printf("[WSc] Disconnected!%s\n", data);
  });
  sock.on("url", [](char* data) {
    Serial.printf("[WSc] URL! '%s'\n", data);
    URL = data;
    stopPlay();
    startPlay();
  });
  sock.begin(wshost, wsport);
}

void loop() {
  if (decoder && decoder->isRunning()) {
    if (!decoder->loop()) {
      stopPlay();
      newUrl = true;
      retryms = millis()+2000;
    }
  }
  if ((millis() > retryms) && newUrl) startPlay();
  sock.loop();
}

