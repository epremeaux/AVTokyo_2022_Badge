## AVTokyo 2022 event badge
![alt_text](images/promo/AFT2022badge_TK.jpg "Badge in front of the TKNightclub neon sign")

### MQTT enabled, Hacker tools, Sensors and Games on ESP32
The AVTokyo 2022 event attendee badge was designed and produced by VPCAsia. Our vision was to provide a useful tool for the attendee not only at the event, but far into the future.

It is based on an 8MB ESP32 with 280x240 pixel TFT full color display (sourced from smart watches). Additionally, it includes a joystick (4 directions plus “press to select”), two game buttons (B and A), and two sensors (Temperature/Humidity and Accelerometer). Finally, the device is fully battery powered, relying on an 18650 style Li-ion battery with associated protections and USB charging.

The software runs a variety of open source Arduino libraries. In particular, we utilized [tcMenu](https://www.thecoderscorner.com/products/arduino-libraries/tc-menu/) and TFT_eSPI to provide all of our software UI, and Adafruit libraries to manage sensors and MQTT (Message Queue, Telemetry Transport).

While WiFi is connected and MQTT messaging is enabled, the badge will send Temperature, Humidity and Battery Voltage measurements to an MQTT server every 5 seconds (the interval is redefinable). Additionally, when you obtain a new best personal score on any game, the new score will be queued to be sent in the next MQTT post. While gaming, MQTT posting is temporarily disabled, in order to provide the best gaming experience.


## Tour of the Board


## Fixes to the production version


## About the Server

On our server side, we used a variety of services. It all starts by receiving the MQTT message in the MQTT server. From here, messages are written to a database using some custom Java.
From this database we then made web pages to output various statistics. In particular, we wanted to make a scoreboard for the two games included with the badge.
![alt_text](images/promo/gamescore.png "Scores at the end of the event")

MQTT is simply a message bucket service. Many other services can subscribe to it. In fact, the code which writes to the database simply subscribes to the MQTT messages. Because this concept is so flexible, other services can easily subscribe at the same time.

Node-Red is a very user friendly GUI based data application builder. It was a simple matter to create a Node-Red flow to subscribe to badge topics using wild cards, allowing us to easily gather temperature measurements from ALL badges at once for example. This same flow outputs a web page with gauges, plots and text data. Node-Red performance is not very quick, but because it is only subscribing to topics (and another service handles database work), it has no impact on MQTT or our database.

### WiFi BugHunt
WE had one more game to play at the event: a WiFi BugHunt! These consisted of an ESP32 with battery pack hidden inside the nightclub (see AVTokyo_2022_Badge/Software/AVT_FindMe_Switcher/). Using the WiFi scanner tool in the badge (or your smartphone), players could seek out the three hidden WiFi access points. Once found, there was a QR code printed on the container of the WiFi AP. Scanning the code with your smartphone would bring you to a guest book web page, where you could sign that you found the AP and leave a comment. To make it more challenging, the APs occasionally dropped AP mode and joined local WiFi to send us an MQTT message containing the battery voltage, so we could monitor the devices in real time.
It was quite entertaining to see all the script attacks people tried on our guestbook page ;)

— more description from Chris —

We can also read messages from the MQTT server using NodeRed. 

## Getting the code up and running
The source code is "almost ready to run". You will need to install an editor such as Visual Studio Code. Within VSCode, add the PlatformIO extension. After opening the sourcecode folder, PlatformIO SHOULD download all the required extensions (mainly the Arduino framework and the ESP32 platform). All the required libraries are already included in the source code folder (.pio/libdeps/esp32dev/).
Depending on your initial VSCode and PlatformIO configuration, you MAY need to add "lorol/LittleFS_esp32@^1.0.6" to the platform.ini file, under the "lib_deps = " heading. If during compile you receive errors concerning LittleFS, either add it to the platform.ini file, or remove it (just add ; to the beginning of the line).

Additionally, you will need to create a "secrets.h" file like so:

//wifi
#define SECRET_SSID "your first AP"
#define SECRET_PASS "AP1 password"

#define SECRET_SSID2 "Second AP"
#define SECRET_PASS2 "AP2 password"

#define SECRET_SSID3 "third AP"
#define SECRET_PASS3 "AP3 password"

//mqtt
#define SECRET_SERVER "MQTT URL or IP" 


You can add as many WiFi access points as you want. Look at the MyWiFi.cpp file to add them to the array. The badge will first try to connect to whatever AP you have saved manually from the menu system. If that connection fails, it will then scan through the list given at the top of MyWiFi.cpp (the list is filled in from the secrets.h file). It will try to match any of these AP names to a scanned list available. If one matches, it will try to connect.

In addition to WiFi, you may want to point the badge to your own MQTT server. You may enter it manually in the menu, but you may prefer to hard code it. We have only defined the SECRET_SERVER keyword in the secrets.h file. This assumes an unsecured connection. So, you may want to modify MQTT_Sensors.cpp and secrets.h such that you can define a username and access key in secrets.h.
