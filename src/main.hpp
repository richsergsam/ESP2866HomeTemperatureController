#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include "AsyncJson.h"
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "ESP8266TimerInterrupt.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#include "HomeTemperatureController.hpp"

/* This is sketch for "smart" home temperature controller on LOLIN(WEMOS) D1 mini Lite (wifi) 
  with relay module and dallas 18b20 thermal sensor.
  The mian idea is turn on or turn of relay by thermal sensor and current time.
  There are two mode to set home temperature:
  1) ALWAYS_MODE - target home temperature is constant;
  2) DAY_NIGHT_MODE - target home temperature depends on current time. In this mode there are 
     two target temperatures (day and night) and two time settings for day start and day end.


  Used libraries:
  * ESP8266WiFi           https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/readme.html
  * ESPAsyncWebServer     https://github.com/me-no-dev/ESPAsyncWebServer
  * NTPClient             https://github.com/arduino-libraries/NTPClient
  * ESP8266TimerInterrupt https://github.com/khoih-prog/ESP8266TimerInterrupt
  * DallasTemperature     https://github.com/milesburton/Arduino-Temperature-Control-Library example  https://espsmart.ru/blog/19-podkljuchenie-datchika-temperatury-ds18b20-k-esp8266.html
  * ArduinoJson           https://arduinojson.org/
*/

//***********************************************************************
// USER SETTINGS START

// Serial speed
const unsigned long SERIAL_SPEED = 230400;

// WIFI settings
const char *WIFI_SSID = "WHITE";
const char *WIFI_PASSWORD = "ShowMustGoOn";

// Temperature sensor correction +/- in celsius degrees.
const float TEMPERATURE_CORRECTION = 1.0;

// Relay module PIN settings
const uint8_t PIN_RELAY = D7;

// DS18B20 temperature sensor PIN settings. In my case GPIO14 (D5)
const uint8_t PIN_DS18B20 = D5;

//TIME ZONE settings
const long TIME_UTC_OFFSET_IN_SECONDS = 4 * 60 * 60;             // time zone +4
const long TIME_SYNCHRONIZATION_INTERVAL_IN_MS = 60 * 60 * 1000; // interval for synchronization time with internet. In my case every 1 hour

// USER SETTINGS END
//***********************************************************************

//Web Server settings
AsyncWebServer server(80);

// Initialise NTPClient for time synchronization
WiFiUDP ntpUDP;
NTPClient timeClient = NTPClient(ntpUDP, "pool.ntp.org", TIME_UTC_OFFSET_IN_SECONDS, TIME_SYNCHRONIZATION_INTERVAL_IN_MS);
ESP8266Timer ITimer;

// Temperature sensor
OneWire oneWire(PIN_DS18B20);
DallasTemperature temperatureSensors(&oneWire);

// For correct work of ESP.getVcc(). It needs for voltage measure at current time.
ADC_MODE(ADC_VCC);

int last_seconds = -1;
HomeTemperatureController *status;

// functions declaration for main.cpp
void servePostForCurrentParams(AsyncWebServerRequest *request, JsonVariant &json);