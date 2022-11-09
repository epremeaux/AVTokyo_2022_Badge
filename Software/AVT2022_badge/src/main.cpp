#include <Arduino.h>
#include "AVT2022_badge_menu.h"
#include "TFT_eSPI.h"

//#include our application header files
#include "variables.h"
#include "MyWiFi.h"
#include "ButtonIcons.h"
#include "BatteryWidget25x15.h"
#include "WiFiWidget28x20.h"
#include "life.h"
#include "scanner.h"
#include "quiz.h"
#include "snek.h"
#include "Plotter.h"
#include "MQTT_Sensors.h"
#include "SaveScreen.h"
#include "Preferences.h"
// images
#include "avtokyo.h"
#include "vpcasia.h"

#define MAX_IMAGE_WIDTH 240
#define MAX_IMAGE_HEIGHT 280

Preferences Global::preferences;
// Note: Key name is limited to 15 chars.
// preferences / user
String UserName;
// preferences / system
int BackLight;
int BadgeMode;

int WifiConnected = false;

int Global::FWversion = 1;

// preferences / games
// preferences / scores

uint16_t Global::TODcounter = 0;

// hardware config
#define RIGHT_PIN 32 // 36
#define DOWN_PIN 33
#define UP_PIN 34
#define LEFT_PIN 35
#define SELECT_PIN 39
#define A_PIN 25 // A button
#define B_PIN 26
#define USR_LED 14

// backlight dimming
#define BACKLIGHT_PIN 27
const int freq = 1000;
const int ledChannel = 0;
const int resolution = 8;

/************ Titlebar Widgets ******************/
//  padding widget to push icons left. Make this the last widget in chain
TitleWidget BatteryWidget(iconsBattery, 5, 15, 25);
TitleWidget WiFiWidget(iconsWiFi, 5, 28, 20, &BatteryWidget);
TitleWidget iconspadWidget(iconsWiFi, 5, 16, 12, &WiFiWidget);


/************ Configuring WiFi ******************/
char Global::WLAN_SSID[]; // changable ssid and pw
char Global::WLAN_PASS[];
bool Global::WiFi_Enable = true; // true = wifi enabled
bool Global::WiFi_Good = false;
bool Global::WiFi_DoReset = true;
bool Global::WiFi_DoDisable = false;
char Global::USER_ID[];


/************ TASK IDs ******************/
taskid_t Global::SenserUpdateTaskID;
taskid_t Global::ConnectWiFiTaskID;
taskid_t Global::PostMQTTTaskID;




/****************************************************************************
 *                      Helper functions
 ****************************************************************************/
void showImages()
{
    gfx.fillScreen(TFT_BLACK);
    gfx.pushImage(0, 0, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, (uint8_t *)avtokyo_logo, true);
    delay(3500);
    gfx.fillScreen(TFT_BLACK);
    gfx.pushImage(0, 0, MAX_IMAGE_WIDTH, MAX_IMAGE_HEIGHT, (uint8_t *)vpcasia_logo, true);
    delay(3500);
    gfx.fillScreen(TFT_BLACK);
}


void ResetScreen(unsigned int encoderValue, RenderPressMode clicked)
{       // draws a screen to tell user the factory defaults have been restored
    if (Global::TODcounter == 0)
    {
        Global::TODcounter++;
        gfx.fillScreen(TFT_BLACK);
        gfx.setTextSize(1);
        gfx.setTextColor(TFT_WHITE);
        gfx.setCursor(50, 10);
        gfx.println("Factory settings");
        gfx.println("         restored");
    }
    delay(1000);
    Global::TODcounter = 0;
    renderer.giveBackDisplay();
}


void WipeAllPrefs()
{
    // clears out all preference namespaces.
    // upon reboot, the defaults will be re-applied
    Global::preferences.begin("User", false);
    Global::preferences.clear();
    Global::preferences.end();

    Global::preferences.begin("Network", false);
    Global::preferences.clear();
    Global::preferences.end();

    Global::preferences.begin("System", false);
    Global::preferences.clear();
    Global::preferences.end();

    Global::preferences.begin("MQTT", false);
    Global::preferences.clear();
    Global::preferences.end();

    Global::preferences.begin("Quiz", false);
    Global::preferences.clear();
    Global::preferences.end();

    Global::preferences.begin("Snek", false);
    Global::preferences.clear();
    Global::preferences.end();
    // nitification screen
    renderer.takeOverDisplay(ResetScreen);
}

void onPressed(uint8_t pin, bool heldDown)
{
    // This function must exist, even if it needs no additional code
    // it allows all of our buttons to be available to decode in games/tools
    // we can also do stuff like below, but this would print on every press.. its better to decode as needed in the game/tool
    // we also MUST have the below class (MyKeyListener), even though its empty.
    // in setup, we will add all the additional buttons we want to listen for and decode

    /* if (pin == A_PIN){
       Serial.println("A button polled");
     }
     if (pin == B_PIN){
       Serial.println("B button polled");
     }  */
}

class MyKeyListener : public SwitchListener
{
public:
    void onPressed(uint8_t pin, bool held) override
    {
    }

    void onReleased(uint8_t /*pin*/, bool held) override
    {
        // Called on key release
    }
} keyListener;



// some vars to cycle title widgets until code written to control them...
int widgetCounter = 0;
bool widgetDirUp = true;

void updateIconsTask()
{
    /* Battery range
     * From 4.25V to 3.6 volts
     * However the screen flickers a LOT and becomes very dim
     * Practical range is from 4.25 to 3.86V
     */
    // cludgy fix for the battery range. Voltage divider is not precision, and ESP32 ADC is terrible
    float BAT_HIGHVAL = 4.25;

    float batVal = Global::BatteryVoltage;
    // cludge gear ENGAGE!!
     if (batVal > BAT_HIGHVAL) {
        batVal = BAT_HIGHVAL;
    }
    int BatSetIcon;
    if(batVal > 4.1) {
        BatSetIcon = 4;
    } else if (batVal > 4.0) {
        BatSetIcon = 3;
    } else if (batVal > 3.8) {
        BatSetIcon = 2;
    } else if (batVal > 3.6) {
        BatSetIcon = 1;
    } else {
        BatSetIcon = 0;
    }
    BatteryWidget.setCurrentState(BatSetIcon);


    int wifiIconVal;
    if (WiFi.status() == WL_CONNECTED) {
        wifiIconVal = map(WiFi.RSSI(), -100, -50, 0, 4);
    } else {
        wifiIconVal = 0;
    }
    WiFiWidget.setCurrentState(wifiIconVal);
    // https://www.netspotapp.com/wifi-signal-strength/what-is-rssi-level.html#:~:text=RSSI%20Level%20Meaning%20Explained,-As%20we've&text=The%20higher%20the%20RSSI%20value,is%20no%20signal%20at%20all.
}

/****************************************************************************
 *                      Menu Callbacks!
 * 
 * These callbacks are used by the menu task when you activate or change
 * a menu item. Not all menu items have callbacks. They set at design time
 * with tcMenuDesigner tool, and defined in _menu.cpp and _menu.h
 * Empty callbacks may be eliminated in the future. Its not always easy
 * to predict up front if you REALLY need a callback or not.
 * 
 ****************************************************************************/

/****************************************************************************
 *                      BASIC SETTINGS (SETUP)
 ****************************************************************************/
void CALLBACK_FUNCTION setBadgeMode(int id)
{
    // empty callback. 
}

void CALLBACK_FUNCTION setBacklight(int id)
{
    BackLight = menuSetupBacklight.getCurrentValue();
    // map 0-9 to 10-255
    int set = map(BackLight, 0, 9, 10, 255);
    ledcWrite(ledChannel, set);
}

void CALLBACK_FUNCTION ResetPreferences(int id)
{
    WipeAllPrefs();
}


/****************************************************************************
 *                      WIFI
 ****************************************************************************/

void CALLBACK_FUNCTION setWiFiEnable(int id)
{
    if (menuSetupWiFiEnableWiFi.getBoolean() == true)
    { 
        Global::WiFi_DoReset = true;
    }
    else
    { 
        Global::WiFi_DoDisable = true;
    }
}

void CALLBACK_FUNCTION setSSID(int id)
{
    // TODO - your menu change code
}

void CALLBACK_FUNCTION setWiFiPassword(int id)
{
    // TODO - your menu change code
}

void CALLBACK_FUNCTION saveWifi(int id)
{
    saveWiFiSettings();

    renderer.takeOverDisplay(SavedScreen);
}

/****************************************************************************
 *                      SETUP SAVE
 ****************************************************************************/

void CALLBACK_FUNCTION SetupSave(int id)
{
    // read the settings from menu
    UserName = menuSetupName.getTextValue();
    BackLight = menuSetupBacklight.getCurrentValue();
    BadgeMode = menuSetupBadgeMode.getCurrentValue();

    // save user settings
    Global::preferences.begin("User", false);
    Global::preferences.putString("UserName", UserName);
    Global::preferences.end();

    // save system settings
    Global::preferences.begin("System", false);
    Global::preferences.putInt("BackLight", BackLight); // write the value
    Global::preferences.putInt("BadgeMode", BadgeMode); // Enable this line AFTER you have the escape back to menu funtions in these modes.
    Global::preferences.end();

    // draw a screen to alert the user that settings are saved.
    renderer.takeOverDisplay(SavedScreen);
}

/****************************************************************************
 *                      MQTT
 ****************************************************************************/

void CALLBACK_FUNCTION setMQTTEnable(int id)
{
    if (menuSetupMQTTEnable.getCurrentValue() == true)
    { // re-enable the wifi connect task
      // crsx     taskManager.setTaskEnabled(Global::PostMQTTTaskID, true);
      //      Serial.println("enabled MQTT post task");
    }
    else
    { // disable the wifi connect task
        taskManager.setTaskEnabled(Global::PostMQTTTaskID, false);
        Serial.println("disabled MQTT post task");
    }
}

void CALLBACK_FUNCTION setMQTTBroker(int id)
{
    // TODO - your menu change code
}

void CALLBACK_FUNCTION setMQTTPort(int id)
{
    // TODO - your menu change code
}

void CALLBACK_FUNCTION setMQTTPostInterval(int id)
{
    // TODO - your menu change code
}

void CALLBACK_FUNCTION saveMQTT(int id)
{
    SaveMQTTsettings();
    renderer.takeOverDisplay(SavedScreen);
}

/****************************************************************************
 *                      GAMES
 ****************************************************************************/
void CALLBACK_FUNCTION LaunchGameOfLife(int id)
{
    Serial.println();
    Serial.println("launch GOL");

    Global::TODcounter = 0;
    renderer.takeOverDisplay(create_life);
}

void CALLBACK_FUNCTION LaunchSnek(int id)
{
    Serial.println();
    Serial.println("launching Snek");

    Global::TODcounter = 0;
    renderer.takeOverDisplay(snek_game);
}


void CALLBACK_FUNCTION launchQuiz(int id)
{
    Global::TODcounter = 0;
    renderer.takeOverDisplay(new_quiz);
}

/****************************************************************************
 *                      TOOLS
 ****************************************************************************/
void CALLBACK_FUNCTION LaunchWiFiScanner(int id)
{
    Global::WiFi_DoDisable = true;
    renderer.takeOverDisplay(new_WiFi_scan);
}

void CALLBACK_FUNCTION launchBLEScan(int id)
{
    Global::WiFi_DoDisable = true;
    renderer.takeOverDisplay(new_Bluetooth_scan);
}

/*******************  BUTTON TESTER  ********************************/

void CALLBACK_FUNCTION LaunchButtonTest(int id)
{
    // ButtonTester app is part of the ButtonIcons.cpp
    renderer.takeOverDisplay(ButtonTester);
}

/****************************************************************************
 *                      SENSORS
 ****************************************************************************/

void CALLBACK_FUNCTION PlotTemperature(int id)
{
    renderer.takeOverDisplay(drawTemperaturePlot);
}

void CALLBACK_FUNCTION PlotHumidity(int id)
{
    renderer.takeOverDisplay(drawHumidityPlot);
}

void CALLBACK_FUNCTION PlotIMU(int id)
{
        renderer.takeOverDisplay(drawIMUPlot);
}

/****************************************************************************
 *                      ABOUT
 ****************************************************************************/
int aboutState = 0;
void AboutCallback(unsigned int encoderValue, RenderPressMode clicked)
{
    // At this point clicked is the status of the select button
    // it can be RPRESS_NONE, RPRESS_PRESSED or RPRESS_HELD
    // encoderValue is the current value of the rotary encoder

    switch (aboutState)
    {
    case 0:
    {
        gfx.fillScreen(TFT_BLACK);
        gfx.setTextSize(1);
        gfx.setTextColor(TFT_WHITE);
        gfx.setCursor(0, 5);
        gfx.setTextColor(TFT_GREEN);
        gfx.print(F("   PCB: "));
        gfx.setTextColor(TFT_WHITE);
        gfx.println(F("Emery Premeaux"));
        gfx.setCursor(0, 35);
        gfx.setTextColor(TFT_RED);
        gfx.print(F("Soft: "));
        gfx.setTextColor(TFT_WHITE);
        gfx.println(F("Emery Premeaux"));
        gfx.setCursor(90, 65);
        gfx.println(F("Taylan Ayken"));
        gfx.setCursor(0, 95);
        gfx.setTextColor(TFT_CYAN);
        gfx.print(F("Network: "));
        gfx.setTextColor(TFT_WHITE);
        gfx.println(F("Chris Shannon"));
        gfx.setCursor(0, 125);
        gfx.setTextColor(TFT_YELLOW);
        gfx.print(F("Solder: "));
        gfx.setTextColor(TFT_WHITE);
        gfx.println(F("Renee Steiner"));
        gfx.setCursor(0, 150);
        gfx.setTextColor(TFT_LIGHTGREY);
        gfx.println(F("      Arduino, TcMenu"));
        gfx.setCursor(0, 180);
        gfx.println(F("     Kicad, Platform.io"));
        gfx.setCursor(0, 210);
        gfx.setTextColor(TFT_RED);
        gfx.println(F("      FLOSS FOREVER!"));
        aboutState++;
    }
    break;

    case 1:
    {
        if (clicked)
        {
            renderer.giveBackDisplay();
            aboutState = 0;
        }
    }
    break;
    }
}

void CALLBACK_FUNCTION about(int id)
{
    renderer.takeOverDisplay(AboutCallback);
}

/****************************************************************************
 *                     Name Badge Mode
 ****************************************************************************/
int NBMState = 0;
void NameBadgeModeCallback(unsigned int encoderValue, RenderPressMode clicked)
{
    // At this point clicked is the status of the select button
    // it can be RPRESS_NONE, RPRESS_PRESSED or RPRESS_HELD
    // encoderValue is the current value of the rotary encoder
    String uName = menuSetupName.getTextValue();

    switch (NBMState)
    {
    case 0:
    {
        gfx.fillScreen(TFT_BLACK);
        gfx.setTextSize(2);
        gfx.setTextColor(TFT_WHITE);
        gfx.setTextDatum(MC_DATUM);
        gfx.drawString(uName, 140, 120);
        gfx.setTextDatum(TL_DATUM);
        NBMState++;
    }
    break;

    case 1:
    {
        if (clicked)
        {
            gfx.setTextSize(1);
            renderer.giveBackDisplay();
            aboutState = 0;
            NBMState = 0;
        }
    }
    break;
    }
}



/****************************************************************************
 *                     SETUP
 ****************************************************************************/
void setup()
{

    pinMode(USR_LED, OUTPUT);
    digitalWrite(USR_LED, HIGH);
    // this is the padding widget to push the menu widgets left (due to the curved corner).
    iconspadWidget.setCurrentState(1);
    Serial.begin(115200);

    Serial.println("In Setup()");

    // pull USER preferences...
    Global::preferences.begin("User", false);
    UserName = Global::preferences.getString("UserName", "Anonymous");
    Global::preferences.end();
    // set values in menu...
    // string to char array..
    int n = UserName.length();
    char namechar[n + 1];
    strcpy(namechar, UserName.c_str());
    menuSetupName.setTextValue(namechar);

    // pull SYSTEM preferences...
    Global::preferences.begin("System", false);
    BackLight = Global::preferences.getInt("BackLight", 5); // 0 indexed. 5 sets to 60%
    BadgeMode = Global::preferences.getInt("BadgeMode", 0);
    Global::Global::preferences.end();
    // map the value for PWM output
    int set = map(BackLight, 0, 9, 10, 255);
    // configure LED PWM functionalitites
    ledcSetup(ledChannel, freq, resolution);
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(BACKLIGHT_PIN, ledChannel);
    ledcWrite(ledChannel, set);
    // set values in menu...
    menuSetupBadgeMode.setCurrentValue(BadgeMode);
    menuSetupBacklight.setCurrentValue(BackLight);

    Serial.println("getting WiFi Setttings");

    getWiFiSettings();
     Global::WiFi_Good = false;


    Serial.println("getting MQTT Settings");

    getMQTTsettings();


    // pull GAME preferences...
    // This is for game settings, scores, etc..
    Global::preferences.begin("Games", false);
    //      most games will get their prefs when launched
    Global::preferences.end();
    // set values in menu...
    Serial.println("Getting UUID");
    getUUID();

    init_TempHumidity();

    // prep menu and additional items

    Serial.println("Setting up switches and menu");

    switches.init(ioUsingArduino(), SWITCHES_NO_POLLING, true);

    Serial.printf("Width: %u  Height: %u \n", gfx.width(), gfx.height());

    gfx.begin();

    showImages();

    delay(500);

    setupMenu();

    // additional switch listeners
    switches.addSwitchListener(A_PIN, &keyListener, 25, false);
    switches.addSwitchListener(B_PIN, &keyListener, 25, false);

    renderer.setFirstWidget(&iconspadWidget);

    /****************************************************************************
     *    configure the device based on the set badge mode
     ****************************************************************************/
    switch (BadgeMode)
    {
    case 0:
    { // ******************** Normal Mode ********************
        // jump straight to the menu
    }
    break;

    case 1:
    { // ******************** Namebadge Mode ********************
        // Take over the display and print the user's name on the screen, with animation
        // wait for button press, then return to normal mode.
        renderer.takeOverDisplay(NameBadgeModeCallback);
    }
    break;
        // add additional badge modes here..
        // ******************** Default ********************
    default:
    {
        // if you got here something went seriously wrong!!
    }
    break;
    }

    // add tasks to taskmanagerIO
    // we dont need an ID for the icon task, as it will always run:
    taskManager.scheduleFixedRate(1, updateIconsTask, TIME_SECONDS);

    // these tasks we may stop and restart later, so we need the task ID
    Global::SenserUpdateTaskID = taskManager.scheduleFixedRate(1, SensorsMenu_Update, TIME_SECONDS);
    Global::ConnectWiFiTaskID = taskManager.scheduleFixedRate(1, Do_WiFi_FSM, TIME_SECONDS);

    if (menuSetupWiFiEnableWiFi.getBoolean() == true)
    { 
        Global::WiFi_DoReset = true;
    }

    Serial.println("Starting Loop...");
    digitalWrite(USR_LED, LOW);
}



/****************************************************************************
 *                     LOOP
 ****************************************************************************/
void loop()
{
    taskManager.runLoop();
}
