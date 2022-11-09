#ifndef GLOBAL_H
#define GLOBAL_H
#include "Preferences.h"
#include "WiFi.h"
#include "AVT2022_badge_menu.h"


struct Global {
    static int FWversion;
    static uint16_t TODcounter;
    static Preferences preferences;

    // task manager IDs
    static taskid_t SenserUpdateTaskID;
    static taskid_t ConnectWiFiTaskID;
    static taskid_t PostMQTTTaskID;


    // Wifi
    static bool WiFi_Enable;        // WiFi should be used
    static bool WiFi_Good;          // its connected to an AP
    static bool WiFi_InUse;         // its being used by another app (not free.. such as doing scanning, or in AP mode)
    static bool WiFi_DoReset;       // request the fsm to reset the wifi
    static bool WiFi_DoDisable;     // request the fsm to disable the wifi
    static char WLAN_SSID[15];
    static char WLAN_PASS[15];
    static char USER_ID[7];         // 6 plus string termination
    static String DefaultWLAN_SSID;      // "TK_SHIBUYA"
    static String DefaultWLAN_PW;  // "TK24002400" 

    // for mqtt:
    static WiFiClient client;
    static bool EnvSensor_ready;    // temperature and humidity sensor initialized and working
    static bool IMU_ready;          // accelerometer initialized and working
    static bool MQTT_Enable;        // do try to post messages
    static bool MQTT_Good;          // was able to connect to the server
    static bool MQTT_GameToPost;    // set true when a game needs to post values
    static int PostInterval;    // value in seconds between posts

    // sensor values
    static float BatteryVoltage;
    static float Temperature;
    static float Humidity;
    static float IMU_x;
    static float IMU_y;
    static float IMU_z;

    #define RIGHT_PIN   32      //36
    #define DOWN_PIN    33
    #define UP_PIN      34
    #define LEFT_PIN    35
    #define SELECT_PIN  39
    #define A_PIN       25      //A button
    #define B_PIN       26
    #define USR_LED     14


};


#endif