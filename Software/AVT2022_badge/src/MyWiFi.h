#include <Arduino.h>
#include "AVT2022_badge_menu.h"


void getUUID();

void getWiFiSettings();
void saveWiFiSettings();
void resetWiFi();        // resets the wifi state machine, to attempt new connection
void disableWiFi();     // to turn off wifi and set state machine to idle
void Do_WiFi_FSM();      // the wifi state machine
