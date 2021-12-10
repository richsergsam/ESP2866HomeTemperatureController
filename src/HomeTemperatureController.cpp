#include "HomeTemperatureController.hpp"

const char *PATH_INDEX_HTML_FILE = "/index.html";
const char *PATH_SETTINGS_FILE = "/settings.json";

HomeTemperatureController::HomeTemperatureController(NTPClient &tc, DallasTemperature &ts, float _t_current_correction)
{
  t_current_correction = _t_current_correction;
  timeClient = &tc;
  temperatureSensors = &ts;

  //Initialize temperature sensor
  temperatureSensors->begin();
  temperatureSensorsCount = temperatureSensors->getDeviceCount();
  if (temperatureSensorsCount > 0)
  {
    temperatureSensors->getAddress(temperatureSensorAddress, 0);
    Serial.println("Temperature sensor initialized");
  }
  else
  {
    Serial.println("ERROR: Temperature sensor initialization failed.");
  }

  //Initialize NTPClient
  timeClient->begin();
  timeClient->forceUpdate();

  //Initialize file system
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
}

void HomeTemperatureController::update_status()
{
  get_t_current_from_sensor();
  if (mode == ALWAYS_MODE)
  {
    update_relay_state(get_t_delta(), t_current, get_t_always());
  }
  if (mode == DAY_NIGHT_MODE)
  {
    //Day mode
    if (isFirstTimeBiggerThanSecond(get_current_time(), get_day_start_time()) &&
        isFirstTimeBiggerThanSecond(get_day_end_time(), get_current_time()))
    {
      update_relay_state(get_t_delta(), t_current, get_t_day());
    }
    //Night mode
    else
    {
      update_relay_state(get_t_delta(), t_current, get_t_night());
    }
  }
}

float HomeTemperatureController::get_t_current_from_sensor()
{
  if (temperatureSensorsCount > 0)
  {
    temperatureSensors->requestTemperatures(); 
    t_current = temperatureSensors->getTempC(temperatureSensorAddress);
    // correct temperature
    t_current = t_current + t_current_correction;
    Serial.print(t_current);
    Serial.println(" C");
  }
  else
  {
    t_current = -1;
  }
  return t_current;
}

void HomeTemperatureController::update_relay_state(float t_delta, float t_current, float t_target)
{
  if (t_current < (t_target - (t_delta / 2.0)))
  {
    set_relay_state(true);
  }
  if (t_current > (t_target + (t_delta / 2.0)))
  {
    set_relay_state(false);
  }
}

void HomeTemperatureController::set_relay_state(bool state)
{
  relay_state = state;
  digitalWrite(relay_pin, relay_state == true ? LOW : HIGH);
}

int *HomeTemperatureController::get_current_time()
{
  current_time[0] = timeClient->getHours();
  current_time[1] = timeClient->getMinutes();
  current_time[2] = timeClient->getSeconds();
  return current_time;
}

void HomeTemperatureController::setErrorMessage(String msg)
{
  errorMessage = timeToString(current_time) + " : " + msg;
  Serial.print("Error: ");
  Serial.println(errorMessage);
}

void HomeTemperatureController::setErrorMessage(String msg, String serial_msg)
{
  setErrorMessage(msg);
  Serial.println(serial_msg);
}

String HomeTemperatureController::getParameters()
{
  String output = "{\"t_current\":" + String(get_t_current()) + "," +
                  "\"t_delta\":" + String(t_delta) + "," +
                  "\"t_always\":" + String(t_always) + "," +
                  "\"t_day\":" + String(t_day) + "," +
                  "\"t_night\":" + String(t_night) + "," +
                  "\"mode\":" + String(mode) + "," +
                  "\"relay_state\":" + String(get_relay_state()) + "," +
                  "\"current_time\":" + timeToJson(get_current_time()) + "," +
                  "\"day_start_time\":" + timeToJson(day_start_time) + "," +
                  "\"day_end_time\":" + timeToJson(day_end_time) + "}";
  //Serial.println(output);
  return output;
}

bool HomeTemperatureController::checkIfContain(DynamicJsonDocument doc, String key)
{
  if (!doc.containsKey(key))
  {
    setErrorMessage("Json document does not contain key: " + key);
    return false;
  }
  return true;
}

bool HomeTemperatureController::setParameters(JsonObject doc)
{
  //DynamicJsonDocument doc(500);
  //deserializeJson(doc, json);
  if (!checkIfContain(doc, "t_delta"))
  {
    return false;
  }
  float t = doc["t_delta"];
  if (t < T_DELTA_MIN || t > T_DELTA_MAX)
  {
    setErrorMessage("t_delta must be from " + String(T_DELTA_MIN) + " to " + String(T_DELTA_MAX) + ". Your is " + String(t));
    return false;
  }
  t_delta = t;

  if (!checkIfContain(doc, "t_always"))
  {
    return false;
  }
  t = doc["t_always"];
  if (t < T_MIN || t > T_MAX)
  {
    setErrorMessage("t_always must be from " + String(T_MIN) + " to " + String(T_MAX) + ". Your is " + String(t));
    return false;
  }
  t_always = t;

  if (!checkIfContain(doc, "t_day"))
  {
    return false;
  }
  t = doc["t_day"];
  if (t < T_MIN || t > T_MAX)
  {
    setErrorMessage("t_day must be from " + String(T_MIN) + " to " + String(T_MAX) + ". Your is " + String(t));
    return false;
  }
  t_day = t;

  if (!checkIfContain(doc, "t_night"))
  {
    return false;
  }
  t = doc["t_night"];
  if (t < T_MIN || t > T_MAX)
  {
    setErrorMessage("t_night must be from " + String(T_MIN) + " to " + String(T_MAX) + ". Your is " + String(t));
    return false;
  }
  t_night = t;

  if (!checkIfContain(doc, "mode"))
  {
    return false;
  }
  int m = doc["mode"];
  if (m != 12 && m != 24)
  {
    setErrorMessage("mode must be 12 or 24. Your is " + String(m));
    return false;
  }
  mode = doc["mode"];

  int *new_day_start_time = new int[3]();
  int *new_day_end_time = new int[3]();
  if (!checkIfContain(doc, "day_start_time"))
  {
    return false;
  }
  JsonArray array = doc["day_start_time"].as<JsonArray>();
  for (int i = 0; i < 3; i++)
  {
    String s = array[i].as<String>();
    if (!isInteger(s))
    {
      setErrorMessage("Error while parse day_start_time: " + s);
      return false;
    }
    new_day_start_time[i] = array[i].as<int>();
  }
  if (!isTimeCorrect(new_day_start_time))
  {
    return false;
  }
  if (!checkIfContain(doc, "day_end_time"))
  {
    return false;
  }
  array = doc["day_end_time"].as<JsonArray>();
  for (int i = 0; i < 3; i++)
  {
    String s = array[i].as<String>();
    if (!isInteger(s))
    {
      setErrorMessage("Error while parse day_end_time: " + s);
      return false;
    }
    new_day_end_time[i] = array[i].as<int>();
  }
  if (!isTimeCorrect(new_day_end_time))
  {
    return false;
  }
  if (isFirstTimeBiggerThanSecond(new_day_start_time, new_day_end_time))
  {
    setErrorMessage("day_start_time must be < day_end_time");
    return false;
  }

  day_start_time = new_day_start_time;
  day_end_time = new_day_end_time;
  return true;
}

void HomeTemperatureController::saveParametersToFile()
{
  Serial.println("Try to save parameters to file: " + String(PATH_SETTINGS_FILE));
  String parameters = getParameters();
  writeFile(parameters, PATH_SETTINGS_FILE);
  Serial.println("Saving parameters to file finished: " + String(PATH_SETTINGS_FILE));
}
void HomeTemperatureController::loadParametersFromFile()
{
  Serial.println("Try to load parameters from file: " + String(PATH_SETTINGS_FILE));
  String json = readFile(PATH_SETTINGS_FILE);
  Serial.println("Content of " + String(PATH_SETTINGS_FILE) + " file :\n" + json);
  StaticJsonDocument<500> doc; 
  deserializeJson(doc, json);
  setParameters(doc.as<JsonObject>());
  Serial.println("Loading parameters from file finished: " + String(PATH_SETTINGS_FILE));
}

bool HomeTemperatureController::isTimeCorrect(int *time)
{
  if (time[0] < 0 || time[0] > 23)
  {
    setErrorMessage("This time have wrong hours number. Yours is: " + String(time[0]));
    return false;
  }
  if (time[1] < 0 || time[1] > 59)
  {
    setErrorMessage("This time have wrong minutes number. Yours is: " + String(time[1]));
    return false;
  }
  if (time[2] < 0 || time[2] > 59)
  {
    setErrorMessage("This time have wrong seconds number. Yours is: " + String(time[2]));
    return false;
  }
  return true;
}