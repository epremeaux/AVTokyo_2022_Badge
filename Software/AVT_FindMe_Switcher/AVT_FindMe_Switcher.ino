/* for some reason, has a difficult time in VScode platform.io.
 *  code works perfect from Arduino ide, but in platform.io, task loop never stops (no respect of interval value)
 *  and it goes crazy.
 *  Also usually locks up for a long time on boot before starting.
 */

#include "TaskManagerIO.h"

// Load Wi-Fi library
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

const int BatteryPin = 34;
const float BatDivScale = 2.13;

// AP stuff
const char* AP_SSID    = "FIND_ME_1";
const char* AP_PASSWORD = "avtokyo2022";
IPAddress IP = IPAddress (10, 10, 2, 6); // curly braces
//IPAddress gateway = IPAddress (10, 10, 2, 8); // not curly braces
IPAddress NMask = IPAddress (255, 255, 255, 0); // not curly braces


// Wifi client stuff
const char* CLIENT_SSID =       "YOURAP";
const char* CLIENT_PASSWORD =   "YOURAPPASSWORD";

// MQTT stuff
#define AIO_SERVER      "YOURMQTTSERVERIP"    // URL or IP address
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// TaskManager stuff
int interval = 30;    // time in seconds to change to client and update mqtt battery voltage

// visual indicator

const int ledPin =  2;
int ledState = LOW; 


bool switchClientMode() {
  // stop AP
   WiFi.disconnect();
 
  // join local wifi
  Serial.println("Switching to client mode");
  uint8_t wifiretry = 30;
  //WiFi.mode(WIFI_STA); //Optional
  WiFi.begin(CLIENT_SSID, CLIENT_PASSWORD);
  Serial.println("\nConnecting");
  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      wifiretry--;
      delay(100);
      if (wifiretry == 0) {
         Serial.println("Giving up on wifi, returning to AP mode");
         return false;
      }
  }
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

bool postMQTT() {
  // post mqtt message
  int8_t ret;
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 1 second
       retries--;
       if (retries == 0) {
         return false;
       }
  }
  Serial.println("MQTT Connected!");
  float voltage = analogRead(BatteryPin) * (3.3/4096) * BatDivScale;
  char topic[255];
  strcpy(topic, "/AVTokyo2022/Find_Me/");
  strcat(topic, AP_SSID);
  strcat(topic, "/Voltage");
  Serial.println("Posting MQTT message");
  Adafruit_MQTT_Publish Voltage = Adafruit_MQTT_Publish(&mqtt, topic);
  Voltage.publish(voltage);
  return true;
}


bool switchAPMode () {
// stop wifi
  WiFi.disconnect();
  // restart AP
  Serial.println("Switching to AP mode");
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  //server.begin();
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP SSID: ");
  Serial.println(AP_SSID);
  Serial.print("AP IP address: ");
  Serial.println(myIP); 
}


void PostMQTTTask() {
  digitalWrite(ledPin, HIGH);
  bool switchedtoclient =   switchClientMode();
  if (switchedtoclient == true) {
    postMQTT();
  }
  switchAPMode();
  Serial.println("-----------------------");
  digitalWrite(ledPin, LOW);
}


void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  switchAPMode();
  taskManager.scheduleFixedRate(interval, PostMQTTTask, TIME_SECONDS);  
}

void loop(){
  taskManager.runLoop();
 
}
