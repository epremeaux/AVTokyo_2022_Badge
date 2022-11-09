#include <Arduino.h>
#include "MQTT_Sensors.h"
#include "AVT2022_badge_menu.h"
#include "SaveScreen.h"
#include "WiFi.h"
#include "Preferences.h"
#include "variables.h"
#include "secrets.h"


char *SSID_list[] = {SECRET_SSID, SECRET_SSID2, SECRET_SSID3 }; // add as many as you want
char *PW_list[] = {SECRET_PASS, SECRET_PASS2, SECRET_PASS3 }; // remember to update WiFiListCount!

const int WiFiListCount = 3;

const String DefaultWLAN_SSID = "APDEFAULT"; 
const String DefaultWLAN_PW = "APDEFPW";  


int retry = 0;            // retry counter
int listedAP = 0;         // which AP in the array are we trying to connect to
int wifiConnectState = 0; // state machine variable

int ntwks = 0;
char networkSSIDs[30][32];


String MACUUID;

// for MQTT....
WiFiClient Global::client;

void getWiFiSettings()
{
    // pull NETWORK settings...
    Global::preferences.begin("Network", false);
    Global::WiFi_Enable = Global::preferences.getBool("WiFi_Enable", true);
    String strWLAN_SSID = Global::preferences.getString("SSID", DefaultWLAN_SSID);
    String strWLAN_PW = Global::preferences.getString("SSID_PW", DefaultWLAN_PW);
    Global::preferences.end();

    // convert strings to char array for WiFi config and menu printing:
    int ssidlen = strWLAN_SSID.length() + 1;
    int pwlen = strWLAN_PW.length() + 1;
    strWLAN_SSID.toCharArray(Global::WLAN_SSID, ssidlen);
    strWLAN_PW.toCharArray(Global::WLAN_PASS, pwlen);
    menuSetupWiFiEnableWiFi.setBoolean(Global::WiFi_Enable);
    menuSetupWiFiSSID.setTextValue(Global::WLAN_SSID);
    menuSetupWiFiPassword.setTextValue(Global::WLAN_PASS);
    menuSetupWiFiPassword.setPasswordField(true);
}

void saveWiFiSettings()
{
    // copy the settings from the menu
    String menuSSID = menuSetupWiFiSSID.getTextValue();
    String menuPW = menuSetupWiFiPassword.getTextValue();
    Serial.println(menuSSID);
    Serial.println(menuPW);
    int ssidlen = menuSSID.length() + 1;
    int pwlen = menuPW.length() + 1;
    menuSSID.toCharArray(Global::WLAN_SSID, ssidlen);
    menuPW.toCharArray(Global::WLAN_PASS, pwlen);
    // save network preferences
    Global::preferences.begin("Network", false);
    Global::preferences.putBool("WiFi_Enable", menuSetupWiFiEnableWiFi.getBoolean());
    Global::preferences.putString("SSID", menuSSID);
    Global::preferences.putString("SSID_PW", menuPW);
    Global::preferences.end();

    Serial.println("Saved WiFi");
    Serial.println(Global::WLAN_SSID);
    Serial.println(Global::WLAN_PASS);
    renderer.takeOverDisplay(SavedScreen);
    // reset the wifi state machine
    wifiConnectState = 0;
}

void getUUID()
{
    unsigned char mac_base[6] = {0};
    esp_efuse_mac_get_default(mac_base);
    esp_read_mac(mac_base, ESP_MAC_WIFI_STA);
    unsigned char mac_local_base[6] = {0};
    unsigned char mac_uni_base[6] = {0};
    esp_derive_local_mac(mac_local_base, mac_uni_base);
    printf("Local Address: ");

    MACUUID = WiFi.macAddress();
    Serial.print("UUID: ");
    Serial.println(MACUUID);
    // lets convert this to a char array and strip the : symbols, take the last 6 characters for a UUID
    // the char array will be used when posting mqtt messages  char USER_ID[6]
    int uidlen = MACUUID.length() + 1;
    char uidtochar[uidlen];
    MACUUID.toCharArray(uidtochar, uidlen);
    int arrayloc = 0;
    for (int i = 8; i <= uidlen; i++)
    {
        if (uidtochar[i] != ':')
        {
            Global::USER_ID[arrayloc] = uidtochar[i];
            arrayloc++;
        }
    }
    Serial.print("USER_ID: ");
    Serial.println(Global::USER_ID);
    menuSetupWiFiUUID.setTextValue(Global::USER_ID);
}

void resetWiFi()
{
   // getWiFiSettings();
    Global::WiFi_Enable = true;
    menuSetupWiFiStatus.setCurrentValue(0);
    menuSetupWiFiIPaddr.setTextValue("");
    wifiConnectState = 0;
    Serial.println("Reset WiFi FSM");
}


void disableWiFi() {
    Serial.println("Shutdown WiFi");
    WiFi.disconnect(true, true);      // turn off wifi, drop client
    menuSetupWiFiStatus.setCurrentValue(0);
    menuSetupWiFiIPaddr.setTextValue("");
      delay(100); //wait for 100 milliseconds
      Global::WiFi_Enable = false;
      Global::WiFi_Good = false;
      wifiConnectState = 8;     // go to the idle state in FSM
      
}

void Do_WiFi_FSM()
{
    /* state machine called from outside to attempt connections, keep wifi alive, etc. Called every second.
     *
     * states:
     * 0 : not initialized, restarting connection after being captured by wifi scanner, etc
     * 1 : attempt to connect to the user set AP.
     * 2 : Connect to AP retry loop (5 times)
     * 3 : attempt to connect to a listed AP (goes back to state 2)
     * 4 : Connected! Do keep alive
     * 5 : failed.
     *
     * The smart move here would be to first check if the AP name even exists!
     *
     */

    if (Global::WiFi_DoReset == true) {
        resetWiFi();
        Global::WiFi_DoReset = false;
    }

    if (Global::WiFi_DoDisable == true) {
        disableWiFi();
        Global::WiFi_DoDisable = false;
    }

    if (Global::WiFi_Enable == true)
    {
        switch (wifiConnectState)
        {
            // not initialized
        case 0:
        {
            Serial.println("int");
            WiFi.disconnect(false, true);      // leave wifi on, but drop old client
            delay(100); //wait for 100 milliseconds
            WiFi.mode(WIFI_STA);
            retry = 0;
            listedAP = 0;
            //getWiFiSettings(); // load the user's AP settings (or the default)
            Global::WiFi_Enable = true;
            Global::WiFi_Good = false;
             menuSetupWiFiStatus.setCurrentValue(2);     // set menu status to "trying.."
            wifiConnectState++;
        }
        break;

        case 1: // do scan get networks
        Serial.println("scan networks");
            ntwks = WiFi.scanNetworks();
            if (ntwks > 0)
            { // found some networks
                Serial.println("WiFi Networks");
                for (int i = 0; i < ntwks; i++)
                {
                    if (i < 30)
                        WiFi.SSID(i).toCharArray(networkSSIDs[i], 32);
                }
                /*for (int i = 0; i < ntwks; i++)
                {
                    Serial.println(networkSSIDs[i]);
                }*/
            }
            wifiConnectState++;
            break;

            // wifi begin user set AP
        case 2:
        {
            Serial.print("connect to ");
            Serial.print("SSID: ");
            Serial.println(Global::WLAN_SSID);
            // Serial.print("Pass: "); Serial.println(Global::WLAN_PASS);
            WiFi.begin(Global::WLAN_SSID, Global::WLAN_PASS);
            wifiConnectState++;
        }
        break;

            // retries of AP connect
        case 3:
        {
            if (retry < 7)
            {
                if (WiFi.status() != WL_CONNECTED)
                {
                    Serial.println("RETRY");
                    retry++;
                }
                else
                {
                    Global::WiFi_Good = true;
                    retry = 0;
                    Serial.println("");
                    Serial.println("Connected");
                    wifiConnectState = 6;
                    menuSetupWiFiSSID.setTextValue(Global::WLAN_SSID);
                    menuSetupWiFiPassword.setTextValue(Global::WLAN_PASS);
                    // menuSetupWiFiStatus.setCurrentValue(true);
                    String ipstring = WiFi.localIP().toString();
                    int n = ipstring.length();
                    char ipchar[n + 1];
                    strcpy(ipchar, ipstring.c_str());
                    menuSetupWiFiIPaddr.setTextValue(ipchar);
                    menuSetupWiFiStatus.setCurrentValue(true);
                }
            }
            else
            { // retries exceeded without a connection
                wifiConnectState++;
            }
        }
        break;

            // Wifi begin a listed AP
        case 4:
        {
            Serial.println("connect to existing network");
            if (ntwks > 0)
            {
                // try to connect to an existing..
                for (int i = 0; i < ntwks; i++)
                {
                    for (int j = 0; j < WiFiListCount; j++)
                    {
                        if (strcmp(networkSSIDs[i], SSID_list[j]) == 0)
                        {
                            strcpy(Global::WLAN_SSID, SSID_list[j]);
                            strcpy(Global::WLAN_PASS, PW_list[j]);
                            menuSetupWiFiSSID.setTextValue(Global::WLAN_SSID);
                            menuSetupWiFiPassword.setTextValue(Global::WLAN_PASS);
                            Serial.print("Trying ");
                            Serial.println(Global::WLAN_SSID);
                            WiFi.begin(Global::WLAN_SSID, Global::WLAN_PASS);
                            delay(5);
                            wifiConnectState = 5;
                            break;
                        }
                        //listedAP = j;
                    }
                    if (wifiConnectState == 5)
                        break;
                }
            }

            if (wifiConnectState == 5)
                break;
/*
            if (listedAP < WiFiListCount)
            {
                // int ssidlen = SSID_list[listedAP].length() + 1;
                // int pwlen = PW_list[listedAP].length() + 1;
                // SSID_list[listedAP].toCharArray(Global::WLAN_SSID, ssidlen);
                // PW_list[listedAP].toCharArray(Global::WLAN_PASS, pwlen);

                Serial.println("Trying SSID Array");
                strcpy(Global::WLAN_SSID, SSID_list[listedAP]);
                strcpy(Global::WLAN_PASS, PW_list[listedAP]);

                Serial.print("SSID: ");
                Serial.println(Global::WLAN_SSID);

                menuSetupWiFiSSID.setTextValue(Global::WLAN_SSID);
                menuSetupWiFiPassword.setTextValue(Global::WLAN_PASS);
                WiFi.begin(Global::WLAN_SSID, Global::WLAN_PASS);
                wifiConnectState++; // go to the check / retry
            }
            else
            {
                wifiConnectState = 7; // no more APs to try.. we failed. :(
            }  */
        }
        break;

            // retries of listed AP connect
        case 5:
        {
            if (retry < 7)
            {
                if (WiFi.status() != WL_CONNECTED)
                {
                    Serial.println("RETRY");
                    retry++;
                }
                else
                {
                    Global::WiFi_Good = true;
                    retry = 0;
                    menuSetupWiFiStatus.setCurrentValue(1);
                    String ipstring = WiFi.localIP().toString();
                    int n = ipstring.length();
                    char ipchar[n + 1];
                    strcpy(ipchar, ipstring.c_str());
                    menuSetupWiFiIPaddr.setTextValue(ipchar);
                    wifiConnectState = 6;
                }
            }
            else
            { // retries exceeded without a connection
                listedAP++;
                retry = 0;
                if (listedAP == WiFiListCount) {
                    // exceeded AP list
                    wifiConnectState = 8;
                } else {
                    wifiConnectState = 4; // go back to fetch the next listed AP
                }
            }
        }
        break;

            // Connected!
        case 6:
        {
            Serial.println("connected!");
            wifiConnectState = 8;
        }
        break;

            // failed all connections
        case 7:
        {
            Serial.println("Sorry. Failed to connect to any WiFi.");
            Global::WiFi_Enable = false;
            wifiConnectState = 8;
            // perhaps stop the task in task manager. Restarts when user sets wifi_enable to true again
        }
        break;

        case 8:
        { // connection idle..
            if (WiFi.status() != WL_CONNECTED) {
                resetWiFi();        // reconnect
            }
        }
        break;
        }
    }
}
