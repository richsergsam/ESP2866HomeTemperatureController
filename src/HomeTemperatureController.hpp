#include <Arduino.h>
#include <NTPClient.h>
#include "LittleFS.h"
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <Helper.hpp>

//STRUCTURE FOR STORING STATISTICS
struct TEMPERATURE
{
  unsigned long EpochTime; //getEpochTime returns the Unix epoch, which are the seconds elapsed since 00:00:00 UTC on 1 January 1970
  float Temperature;
};

//STATUS
enum { ALWAYS_MODE = 24,
       DAY_NIGHT_MODE = 12
};

//TEMPERATURE MIN AND MAX SETTINGS
const float T_MIN = 16.0;
const float T_MAX = 30.0;
const float T_DELTA_MIN = 0.1;
const float T_DELTA_MAX = 5.0;

//File system
extern const char *PATH_INDEX_HTML_FILE;
extern const char *PATH_SETTINGS_FILE;

class HomeTemperatureController
{
public:
  HomeTemperatureController(NTPClient &tc, DallasTemperature &ts, float t_current_correction);
  String getErrorMessage() { return errorMessage; }
  void setErrorMessage(String msg);
  void setErrorMessage(String msg, String serial_msg);
  void update_status();
  bool setParameters(JsonObject doc);
  String getParameters();
  void saveParametersToFile();
  void loadParametersFromFile();

private:
  //TEMPERATURE
  DallasTemperature *temperatureSensors;
  int temperatureSensorsCount = 0;
  DeviceAddress temperatureSensorAddress;
  float t_delta = 1;
  float t_always = 23;
  float t_day = 23;
  float t_night = 23;
  float t_current = -1;
  float t_current_correction = 0;
  //MODE
  int mode = ALWAYS_MODE;
  //RELAY
  bool relay_state = LOW;
  uint8_t relay_pin = D7;
  //TIME
  NTPClient *timeClient;
  int *current_time = new int[3]{0, 0, 0};
  int *day_start_time = new int[3]{6, 0, 0};
  int *day_end_time = new int[3]{20, 0, 0};
  //Error message
  String errorMessage = "";

  //TODO: move functions body from cpp to h
  float get_t_delta() { return t_delta; };
  float get_t_always() { return t_always; };
  float get_t_day() { return t_day; };
  float get_t_night() { return t_night; };
  float get_t_current() { return t_current; };
  float get_t_current_from_sensor();
  int get_mode() { return mode; };

  bool get_relay_state() { return relay_state; };
  void set_relay_state(bool state);
  void update_relay_state(float t_delta, float t_current, float t_target);

  int *get_current_time();
  int *get_day_start_time() { return day_start_time; };
  int *get_day_end_time() { return day_end_time; };
  bool isTimeCorrect(int *time);

  bool checkIfContain(DynamicJsonDocument doc, String key);
};
