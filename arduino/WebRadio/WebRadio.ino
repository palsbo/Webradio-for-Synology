/*
  The following Audio library can be found here: https://github.com/earlephilhower/ESP8266Audio/blob/master/src/AudioFileSourceICYStream.h
  Thanks to the author.
*/
//#define DAC   //  uncomment if I2S DAC module is used

#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#ifdef DAC
#include "AudioOutputI2S.h"   //  uncomment if I2S DAC module is connected
#else
#include "AudioOutputI2SNoDAC.h"  //  comment if I2S DAC module is connected
/*
 * if I2S DAC module is not used, use  #include "AudioOutputI2SNoDAC.h" to get sound from pin RX on ESP8266
 */
#endif
#include <ESP8266WiFi.h>
#include "sock.h"
#include <init.h>

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

AudioGeneratorMP3 *mp3;
AudioFileSourceHTTPStream *file;
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

void MDCallback(void *cbData, const char *type, bool isUnicode, const char *string) {
  /*
    const char *ptr = reinterpret_cast<const char *>(cbData);
    (void) isUnicode; // Punt this ball for now
    // Note that the type and string may be in PROGMEM, so copy them to RAM for printf
    char s1[32], s2[64];
    strncpy_P(s1, type, sizeof(s1));
    s1[sizeof(s1) - 1] = 0;
    strncpy_P(s2, string, sizeof(s2));
    s2[sizeof(s2) - 1] = 0;
    Serial.printf("METADATA(%s) '%s' = '%s'\n", ptr, s1, s2);
    Serial.flush();
  */
}

void StatusCallback(void *cbData, int code, const char *string) {
  /*
    const char *ptr = reinterpret_cast<const char *>(cbData);
    // Note that the string may be in PROGMEM, so copy it to RAM for printf
    char s1[64];
    strncpy_P(s1, string, sizeof(s1));
    s1[sizeof(s1) - 1] = 0;
    Serial.printf("STATUS(%s) '%d' = '%s'\n", ptr, code, s1);
    Serial.flush();
  */
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

  file = new AudioFileSourceICYStream();
  file->RegisterMetadataCB(MDCallback, (void*)"ICY");
  buff = new AudioFileSourceBuffer(file, 2048);
  buff->RegisterStatusCB(StatusCallback, (void*)"buffer");
  mp3 = new AudioGeneratorMP3();
  mp3->RegisterStatusCB(StatusCallback, (void*)"mp3");
  #ifdef DAC
  out = new AudioOutputI2S();
  #else
  out = new AudioOutputI2SNoDAC();
  #endif

  sock.on("connect", [](char* data) {
    Serial.printf("[WSc] Connected! %s\n", data);
  });
  sock.on("disconnect", [](char* data) {
    Serial.printf("[WSc] Disconnected!%s\n", data);
  });
  sock.on("url", [](char* data) {
    Serial.printf("[WSc] URL! '%s'\n", data);
    URL = data;
    if (mp3->isRunning()) {
      if (mp3) mp3->stop();
      if (file) file->close();
      delay(1000);
    }
    file->open(&URL[0]);
    mp3->begin(buff, out);
  });
  sock.on("StreamTitle", [](char* data) {
    Serial.printf("[WSc] Title! '%s'\n", data);
  });
  sock.begin(wshost, wsport);
}

uint32_t mytimer;

void loop() {
    if (mp3->isRunning()) {
      if (!mp3->loop()) {
        mp3->stop();
        mp3->begin(buff, out);
      }
    }
  sock.loop();
}

