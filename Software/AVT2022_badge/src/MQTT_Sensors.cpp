#include "MQTT_Sensors.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "Preferences.h"
#include "AVT2022_badge_menu.h"
#include "variables.h"
#include "SaveScreen.h"
#include "secrets.h"

// temperature humidity sensor
#include "Adafruit_SHTC3.h"
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

// Accelerometer
#include "kxtj3-1057.h"
#include "Wire.h"
float sampleRate = 6.25; // HZ - Samples per second - 0.781, 1.563, 3.125, 6.25, 12.5, 25, 50, 100, 200, 400, 800, 1600Hz
uint8_t accelRange = 2;  // Accelerometer range = 2, 4, 8, 16g
KXTJ3 myIMU(0x0E);       // Address can be 0x0E or 0x0F

#define BAT_SENSE 36           // ADC1 channel 0
const float BAT_SCALE = 2.25;   // ADC voltage * BAT_SCALE = real voltage.


bool Global::EnvSensor_ready; // temperature and humidity sensor initialized and working
bool Global::IMU_ready;       // accelerometer initialized and working
float Global::BatteryVoltage;
float Global::Temperature;
float Global::Humidity;
float Global::IMU_x;
float Global::IMU_y;
float Global::IMU_z;
bool Global::MQTT_GameToPost;
int Global::PostInterval = 5;
bool Global::MQTT_Enable; // to post or not to post? that is the question..
bool Global::MQTT_Good;   // true: mqtt connection is working
int rate = 1000;
int MQTTinterval = 1000;

/******************* MQTT basic config **************************/
#define Def_AIO_SERVERPORT 1883         // use 8883 for SSL
#define AIO_USERNAME ""
#define AIO_KEY ""

String AIO_SERVER = ""; 
String AIO_SERVERPORT = "1883";
const int MQTTInterval = 5; // 5 seconds

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&Global::client, SECRET_SERVER, Def_AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/************ Subscribes ************/
// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/avtokyo2022/onoff");

/************ topic feeds ************/
// ultimately:  append /uuid  like : /AVTokyo2022/[UUID]/Sensors/Test/Voltage
// Adafruit_MQTT_Publish VOLT = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/AVTokyo2022/Sensors/Voltage");
// Adafruit_MQTT_Publish TEMPERATURE = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/AVTokyo2022/Sensors/Temperature");
// Adafruit_MQTT_Publish HUMIDITY = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/AVTokyo2022/Sensors/Humidity");

// game feeds
//Adafruit_MQTT_Publish QUIZ = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/AVTokyo2022/Games/Quiz");
//Adafruit_MQTT_Publish SNEK = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/AVTokyo2022/Games/Snek");

/******************  initialize hardware **************************/

bool init_TempHumidity()
{
    // start Temperature / Humidity sensor
    if (!shtc3.begin())
    {
        Serial.println("Couldn't find SHTC3");
        Global::EnvSensor_ready = false;
        return false;
    }
    Serial.println("Found SHTC3 sensor");
    Global::EnvSensor_ready = true;
    return true;
}

bool init_IMU()
{
    if (myIMU.begin(sampleRate, accelRange) != 0)
    {
        Serial.print("Failed to initialize IMU.\n");
        Global::IMU_ready = false;
        return false;
    }
    else
    {
        Serial.print("IMU initialized.\n");
        // Detection threshold, movement duration and polarity
        myIMU.intConf(123, 1, 10, HIGH);
        uint8_t readData = 0;
        // Get the ID:
        myIMU.readRegister(&readData, KXTJ3_WHO_AM_I);
        Serial.print("IMU address: 0x");
        Serial.println(readData, HEX);
        Global::IMU_ready = true;
        return true;
    }
}

/******************  Updating sensors **************************/

bool envSensor_Update()
{
    sensors_event_t humidity, temp;
    Global::EnvSensor_ready = shtc3.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
    Global::Temperature = temp.temperature;
    Global::Humidity = humidity.relative_humidity;
    if (Global::EnvSensor_ready == true)
    {
        return true;
    }
    else
    {
        Global::Temperature = 0;
        Global::Humidity = 0;
        return false;
    }
}

void batterySensor_Update()
{
    /* battery measurement:
     * Voltage divider is set to divide by 0.75
     * 4.1 V Bat = 3.075V ADC
     * 3.0 V Bat = 2.25V ADC
     * https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
     */

    int adctemp = analogRead(BAT_SENSE);
    float adcVolt = adctemp * (3.3 / 4096);

    //       Serial.print("adc: "); Serial.print(adctemp); Serial.print("   "); Serial.print("adc volt: "); Serial.println(adcVolt);
    Global::BatteryVoltage = adcVolt * BAT_SCALE;
    menuSensorsBatteryVoltage.setFloatValue(Global::BatteryVoltage);
    //       Serial.print("battery: "); Serial.println(Global::BatteryVoltage);
    //       Serial.println();
}

void IMU_Update()
{
    if (Global::IMU_ready == false)
    {
        // give it another try...
        init_IMU();
    }
    myIMU.standby(false);
    Global::IMU_x = myIMU.axisAccel(X);
    Global::IMU_y = myIMU.axisAccel(Y);
    Global::IMU_z = myIMU.axisAccel(Z);
    myIMU.standby(true);
}

/******************  sending data to the menu **************************/

void SensorsMenu_Update()
{
    batterySensor_Update();
    menuSensorsBatteryVoltage.setFloatValue(Global::BatteryVoltage);

    // try the env sensor
    //   if (Global::EnvSensor_ready == false) {
    // give it another try...
    //       init_TempHumidity();
    //   }
    envSensor_Update(); // will fill temperature and humidity with zero if read fails, so we can update menu regardless.
    menuSensorsTemperature.setFloatValue(Global::Temperature);
    menuSensorsHumidity.setFloatValue(Global::Humidity);
}

void getMQTTsettings()
{

    // pull MQTT settings
    Global::preferences.begin("MQTT", false);
    Global::MQTT_Enable = Global::preferences.getBool("MQTTenable", true);
    AIO_SERVER = Global::preferences.getString("MQTTbroker", AIO_SERVER);
    AIO_SERVERPORT = Global::preferences.getInt("MQTTport", 1883);
    MQTTinterval = Global::preferences.getInt("MQTTinterval", MQTTInterval);
    Global::preferences.end();
    Serial.print("mqtt port: ");
    Serial.println(AIO_SERVERPORT);
    // string to char array..
    int mqttservlen = AIO_SERVER.length() + 1;
    char brokerchar[mqttservlen];
    AIO_SERVER.toCharArray(brokerchar, mqttservlen);
    strcpy(brokerchar, AIO_SERVER.c_str());
    // set menu text
    menuSetupMQTTEnable.setBoolean(Global::MQTT_Enable);
    menuSetupMQTTBroker.setTextValue(brokerchar);
    menuSetupMQTTPort.setCurrentValue(AIO_SERVERPORT.toInt());
    menuSetupMQTTPostInterval.setCurrentValue(MQTTinterval);
}

void SaveMQTTsettings()
{
    Serial.println();
    Serial.println("Saving MQTT config");
    // get values from the menu items
    Global::MQTT_Enable = menuSetupMQTTEnable.getBoolean();
    AIO_SERVER = menuSetupMQTTBroker.getTextValue();
    AIO_SERVERPORT = menuSetupMQTTPort.getCurrentValue();
    MQTTinterval = menuSetupMQTTPostInterval.getCurrentValue();
    // save to prefs
    Global::preferences.begin("MQTT", false);
    Global::preferences.putBool("MQTTenable", Global::MQTT_Enable);
    Serial.println("MQTTenable");
    Global::preferences.putString("MQTTbroker", AIO_SERVER);
    Serial.println("server");
    Global::preferences.putInt("MQTTport", AIO_SERVERPORT.toInt());
    Serial.println("port");
    Global::preferences.putInt("MQTTinterval", MQTTinterval);
    Serial.println("interval");
    Global::preferences.end();
    renderer.takeOverDisplay(SavedScreen);
}

/******************  Posting to MQTT **************************/

bool postMQTTenv()
{
    char topic[255];

    strcpy(topic, "/AVTokyo2022/[");
    strcat(topic, Global::USER_ID);
    strcat(topic, "]/Sensors/Voltage");

    Adafruit_MQTT_Publish Voltage = Adafruit_MQTT_Publish(&mqtt, topic);
    Voltage.publish(Global::BatteryVoltage);

    strcpy(topic, "/AVTokyo2022/[");
    strcat(topic, Global::USER_ID);
    strcat(topic, "]/Sensors/Temperature");

    Adafruit_MQTT_Publish TempC = Adafruit_MQTT_Publish(&mqtt, topic);
    TempC.publish(Global::Temperature);

    strcpy(topic, "/AVTokyo2022/[");
    strcat(topic, Global::USER_ID);
    strcat(topic, "]/Sensors/Humidity");

    Adafruit_MQTT_Publish Humid = Adafruit_MQTT_Publish(&mqtt, topic);
    return Humid.publish(Global::Humidity);
}

bool postMQTTgames()
{
    char topic[255];

    Global::preferences.begin("Quiz", false);
     if ((Global::preferences.getBool("PostQueue")) == true) {
         int quizscore = Global::preferences.getInt("Score");
         // post mqtt messages
        strcpy(topic, "/AVTokyo2022/[");
        strcat(topic, Global::USER_ID);
        strcat(topic, "]/Games/Quiz");

        Adafruit_MQTT_Publish Quiz = Adafruit_MQTT_Publish(&mqtt, topic);
        Quiz.publish(quizscore);
        Global::preferences.putBool("PostQueue",false);
     }
     Global::preferences.end(); 

     Global::preferences.begin("Snek", false);
     if ((Global::preferences.getBool("PostQueue")) == true) {
         int snekscore = Global::preferences.getInt("Score");
         Serial.print("score: "); Serial.println(snekscore); 
         // post mqtt messages
        strcpy(topic, "/AVTokyo2022/[");
        strcat(topic, Global::USER_ID);
        strcat(topic, "]/Games/Snek");

        Adafruit_MQTT_Publish Snek = Adafruit_MQTT_Publish(&mqtt, topic);
        Snek.publish(snekscore);         
         Global::preferences.putBool("PostQueue",false);
     }
     Global::preferences.end();

    return false;
}

void setupMQTT();


/*************************** MQTT_CONNECT ************************************/
int mqttRetry = 5;
int8_t err;

bool MQTT_connect()
{
    bool ToRet;

    // check connection.
    if (mqtt.connected())
    {
        return true;
    }

    if ((err = mqtt.connect()) != 0)
    { // Report any errors
        Serial.println(mqtt.connectErrorString(err));
        mqtt.disconnect();
        mqttRetry--;
        ToRet = false;
    }
    else
    { // MQTT connection is good
        Serial.println("MQTT Connected");
        menuSetupMQTTStatus.setCurrentValue(1);
        mqttRetry = 5;
        ToRet = true;
        // setupMQTT();
    }

    if (mqttRetry <= 0)
    { // too many retries. Give up and disable MQTT
        Serial.println("Quit trying MQTT");
        menuSetupMQTTStatus.setCurrentValue(0);
        Global::MQTT_Enable = false;
        ToRet = false;
    }
    return ToRet;
}

/*************************** do MQTT post ************************************
 *
 * This function will be called by task or main loop. It will manage the
 * MQTT connection itself, and disable MQTT if it cannot make a stable connection
 *****************************************************************************/
void do_MQTT_post()
{

    if (Global::TODcounter > 0)
        return; // we are gaming, get out.

    if (WiFi.status() == WL_CONNECTED) // ONLY DO THIS IF WIFI IS CONNECTED...
    {                                  // if Wifi is connected, AND MQTT is enabled....
        if (Global::MQTT_Enable == true)
        {

            bool res = MQTT_connect(); // check the mqtt connection. Connect if necessary...

            if (res == true)
            {

                if (postMQTTenv())
                {
                    Serial.println("Sent Sensor Data.");
                    postMQTTgames();
                }

               /* if (Global::MQTT_GameToPost == true)
                { // if there is a game score to post...
                    Serial.println("Trying MQTT post game scores");
                    postMQTTgames();
                }*/
            }
        }
    }
}
