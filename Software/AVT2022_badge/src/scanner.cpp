#include "scanner.h"
#include "ButtonIcons.h"
#include "MyWiFi.h"
#include "variables.h"
#include "AVT2022_badge_menu.h"

/* Scanners need to stop mqtt posting, so we use the frame counter
 * (TODcounter). This counter warns the MQTT loop not to post.
 * Since we are using it, MQTT will NOT post while running scanners.
 * However we should reset TODcounter to 0 on exit, so MQTT will resume.
 */


#define RIGHT_PIN   32      //36
#define DOWN_PIN    33
#define UP_PIN      34
#define LEFT_PIN    35
#define SELECT_PIN  39
#define A_PIN       25      //A button
#define B_PIN       26

// Frames per second
#define FPS 10


/* convert code to be driven by state machine using the state variable
 * 0 : first time : capture wifi
 * 1 : instructions -> render header
 * 2 : scan and show
 * 3 : wait for input : user options here are up, down, rescan, or quit.
 * 4 : ask are you sure you want to quit
 * 5 : wait for user input : quit or re-scan
 * 6 : release wifi / exit
***********************************************************************/


/**********************************************************************
 * Wi-fi scan tool originally by Peter Clement
 * https://www.hackster.io/peterclement/wi-fi-scan-b5b0ed
 * 
 * Modified for AVT-Badge by DIYSCIBORG (Emery)
 **********************************************************************/
#include "WiFi.h"

int listPos;
int network;
uint8_t scannerstate = 0;


void eraseWifiList(int pos, int networkCount) {
  gfx.setTextColor(TFT_BLACK);
  gfx.setCursor(0, 60);
  for (int i = pos; i < networkCount; ++i) {
    gfx.setTextColor(TFT_BLACK);
    gfx.print(WiFi.RSSI(i)); gfx.print(" "); gfx.println(WiFi.SSID(i));
  }
}


void writeWifiList(int pos, int networkCount) {
  gfx.setCursor(0, 60);
  for (int i = pos; i < networkCount; ++i) {
    gfx.setTextColor(TFT_GREEN);
    gfx.print(WiFi.RSSI(i)); gfx.print(" "); gfx.setTextColor(TFT_WHITE); gfx.println(WiFi.SSID(i));
  }
}

void new_WiFi_scan(unsigned int encoderValue , RenderPressMode clicked) {

  switch (scannerstate) {
// 0 : first time : capture wifi  
    case 0: {
      Global::TODcounter++;     // used to bypass mqtt posting
      //disableWiFi();      // shut down wifi
      WiFi.mode(WIFI_STA);
      //Disconnect the WiFi from any existing connection
      WiFi.disconnect();      // retain old client settings
      delay(100); //wait for 100 milliseconds
      scannerstate++;
    } break;
// 1 : instructions
    case 1: {
      gfx.fillScreen(TFT_BLACK);
      gfx.setTextSize(1);
      gfx.setTextColor(TFT_CYAN);
      gfx.setCursor(20, 5);
      gfx.println(F("WiFi Scanner Tool"));
      gfx.println();
      gfx.setTextColor(TFT_WHITE);

      int joy1y = 40;
      draw_Joy(5, joy1y, 6);
      gfx.setCursor(110, joy1y + 30);
      gfx.print("List up/down");

      int btny = joy1y + 80 + 10;
      draw_B(20, btny, false);
      gfx.setCursor(110, btny + 10);
      gfx.print("Scan Again");

      int txty = btny + 60;
      gfx.setCursor(10, txty);
      gfx.println(F("More details in terminal."));

      delay(3000);
      // "overtypes" the original message with black, effectively erasing it..
      gfx.setTextColor(TFT_BLACK);
      gfx.setCursor(110, joy1y + 30);
      gfx.print("List up/down");
      // print the new text
      gfx.setTextColor(TFT_WHITE);
      draw_Joy(5, joy1y, 5);
      gfx.setCursor(110, joy1y + 30);
      gfx.print("Quit");
      delay(3000);
      scannerstate++;
    } break;
// 2 : scan and show
    case 2: {
      network = WiFi.scanNetworks();
      gfx.fillScreen(TFT_BLACK);
      gfx.setTextSize(1);
      gfx.setTextColor(TFT_CYAN);
      gfx.setCursor(20, 5);
      gfx.println(F("WiFi Scanner Tool"));
      gfx.setTextColor(TFT_GREEN);
      gfx.print(F("RSSI "));
      gfx.setTextColor(TFT_WHITE);
      gfx.println(F(" SSID"));
      gfx.drawLine(0, 55, 280, 55, TFT_CYAN);

      if (network == 0) {
        gfx.setTextColor(TFT_RED);
        gfx.println(F("No Networks Found"));
        Serial.println("No Networks Found");
      }
      else {
        listPos = 0;
        writeWifiList(listPos, network);
        for (int i = 0; i < network; ++i) {
          Serial.print(i + 1);
          Serial.print(":");
          //prints the Individual SSID
          Serial.print(WiFi.SSID(i));
          //prints the Signal Strenght
          Serial.println(WiFi.RSSI(i));
          //This line helps to display the Network Encryption Mode whether it's OPEN or has PASSWORD
          //The Tenary Operation checks the Wifi Encryption to see if its OPEN then perfoms commands
          Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "   OPEN ":"   Password Required");
        }
        Serial.printf("printed %u networks \n", network);
        Serial.println("**Scan Complete**");
        Serial.println("    ");
      }
      scannerstate++;
    } break;

// 3 : wait for input : user options here are up, down, rescan, or quit.
    case 3: {
      if (clicked) {        // quit
        scannerstate = 0;   // reset the state or you'll just loop back out when restarting from the menu
        listPos = 0;
        Global::TODcounter = 0;     // used to enable mqtt posting again
        if (menuSetupWiFiEnableWiFi.getBoolean() == true) {
          Global::WiFi_DoReset = true;
        }
        renderer.giveBackDisplay();
      }
      else if (switches.isSwitchPressed(B_PIN)){    // rescan
        delay(100); 
        eraseWifiList(listPos, network);
        network = 0;
        listPos = 0;
        scannerstate = 2;
      }
      else if (switches.isSwitchPressed(UP_PIN)){   // list up
        //Serial.printf("UP: %u to %u \n", listPos, network);
        if (listPos > 0) {
          eraseWifiList(listPos, network);
          listPos = listPos - 1;
          writeWifiList(listPos, network);
        }
      }
      else if (switches.isSwitchPressed(DOWN_PIN)){   // list down
        //Serial.printf("Down : %u to %u \n", listPos, network);
        if (listPos < network -1) {
          eraseWifiList(listPos, network);
          listPos = listPos + 1;
          writeWifiList(listPos, network);
        }
      }
    } break;
      default:
          // if you got here something went seriously wrong
      break;
  }
}



/**********************************************************************
 * BLE scanner Based on Neil Kolban example for IDF
 * Ported to Arduino ESP32 by Evandro Copercini
 * https://randomnerdtutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
 * 
 * Modified for AVT-Badge by DIYSCIBORG (Emery)
 **********************************************************************/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 3; //In seconds
int scanWindow = 15;
BLEScan* pBLEScan;
int BTScanState = 0;



class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      int rssi = advertisedDevice.getRSSI();
      BLEAddress addr = advertisedDevice.getAddress();
      gfx.setTextColor(TFT_YELLOW);
      gfx.print(rssi); gfx.print(" "); 
      gfx.setTextColor(TFT_WHITE);
      gfx.println(addr.toString().c_str());
    }
};


void new_Bluetooth_scan(unsigned int encoderValue , RenderPressMode clicked) {
  switch (BTScanState) {
// 0 : first time : prep bluetooth 
    case 0: {
      Global::TODcounter++;     // used to bypass mqtt posting
      disableWiFi();
      BLEDevice::init("");
      BTScanState++;
    } break;

// 1 : show instructions
    case 1: {
      
      //disableWiFi();      // shut down wifi
      gfx.fillScreen(TFT_BLACK);
      gfx.setTextSize(1);
      gfx.setTextColor(TFT_CYAN);
      gfx.setCursor(10, 5);
      gfx.println(F("Bluetooth Scanner Tool"));
      gfx.println();
      gfx.setTextColor(TFT_WHITE);

      int32_t btny = 40;
      int32_t joy1y = btny + 60;

      draw_B(20, btny, false);
      gfx.setCursor(110, btny + 10);
      gfx.print("Scan Again");

      gfx.setTextColor(TFT_WHITE);
      draw_Joy(5, joy1y, 5);
      gfx.setCursor(110, joy1y + 30);
      gfx.print("Quit");

      int32_t txty = joy1y + 90;
      gfx.setCursor(10, txty);
      gfx.println(F("More details in terminal."));

      delay(3000);
      BTScanState++;
    } break;

// 2 : scan and show
    case 2: {
      gfx.fillScreen(TFT_BLACK);
      gfx.setTextSize(1);
      gfx.setTextColor(TFT_CYAN);
      gfx.setCursor(20, 5);
      gfx.println(F("Bluetooth Scan"));
      gfx.setTextColor(TFT_YELLOW);
      gfx.print(F("RSSI "));
      gfx.setTextColor(TFT_WHITE);
      gfx.println(F(" Address"));
      gfx.drawLine(0, 55, 280, 55, TFT_CYAN);

  
      // Retrieve a Scanner and set the callback we want to use to be informed when we
      // have detected a new device.  Specify that we want active scanning and start the
      // scan to run for 5 seconds.
      
      Serial.println("Scanning Bluetooth...");
      BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
      pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
      //pBLEScan->setInterval(100);
      pBLEScan->setWindow(scanWindow);  // less or equal setInterval value
      BLEScanResults foundDevices = pBLEScan->start(scanTime);
      Serial.println("Scan done!");
      gfx.setTextColor(TFT_GREEN);
      gfx.println("Scan done");
      pBLEScan->clearResults();
      BTScanState++;
    } break;

// 3 : wait for input : user options here are up, down, rescan, or quit.
    case 3: {
      if (clicked) {
        BTScanState = 0;      // reset the state or you'll just loop back out when restarting from the menu
        //pBLEScan->clearResults();
        Global::TODcounter = 0;     // used to enable mqtt posting again
         if (menuSetupWiFiEnableWiFi.getBoolean() == true) {
          Global::WiFi_DoReset = true;
        }
        renderer.giveBackDisplay();
      }

    else if (switches.isSwitchPressed(B_PIN)) {
        //pBLEScan->clearResults();
        BTScanState = 2;
      }
    } break;

// if you got here something went seriously wrong
    default: {
    } break;
  }
}
