AVTokyo 2022 event badge
MQTT enabled, Hacker tools, Sensors and Games on ESP32



The AVTokyo 2022 event attendee badge was designed and produced by VPCAsia. Our vision was to provide a useful tool for the attendee not only at the event, but far into the future.

It is based on an 8MB ESP32 with 280x240 pixel TFT full color display (sourced from smart watches). Additionally, it includes a joystick (4 directions plus “press to select”), two game buttons (B and A), and two sensors (Temperature/Humidity and Accelerometer). Finally, the device is fully battery powered, relying on an 18650 style Li-ion battery with associated protections and USB charging.

The software runs a variety of open source Arduino libraries. In particular, we utilized tcMenu and TFT_eSPI to provide all of our software UI, and Adafruit libraries to manage sensors and MQTT (Message Queue, Telemetry Transport).

While WiFi is connected and MQTT messaging is enabled, the badge will send Temperature, Humidity and Battery Voltage measurements to an MQTT server every 5 seconds (the interval is redefinable). Additionally, when you obtain a new best personal score on any game, the new score will be queued to be sent in the next MQTT post. While gaming, MQTT posting is temporarily disabled, in order to provide the best gaming experience.

Tour of the Board




Fixes to the production version


About the Server
On our server side, we used a variety of services. It all starts by receiving the MQTT message in the MQTT server. From here, messages are written to a database using some custom Java.
— more description from Chris —

We can also read messages from the MQTT server using NodeRed. 

