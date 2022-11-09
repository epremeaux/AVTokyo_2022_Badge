#include <Arduino.h>


bool init_TempHumidity() ;
bool init_IMU() ;
void SensorsMenu_Update() ;
bool envSensor_Update();

void getMQTTsettings();
void SaveMQTTsettings();

bool MQTT_connect();
void do_MQTT_post() ;

void IMU_Update();
