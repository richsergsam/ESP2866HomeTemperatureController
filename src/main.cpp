#include <main.hpp>

void setup()
{

  Serial.begin(SERIAL_SPEED);
  pinMode(PIN_RELAY, OUTPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  delay(500);

  status = new HomeTemperatureController(timeClient, temperatureSensors, TEMPERATURE_CORRECTION);
  status->loadParametersFromFile();

  server.on("/current_params", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "application/json", status->getParameters()); });
  server.on("/board_status", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", getBoardStatus()); });

  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/current_params", servePostForCurrentParams);
  server.addHandler(handler);

  server.on("/error", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", status->getErrorMessage()); });

  server.onNotFound([](AsyncWebServerRequest *request)
                    { request->send(404, "text/plain", "Page not found"); });

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  server.begin();
  Serial.println("");
  Serial.println("HTTP server started");

  timeClient.update();
  last_seconds = timeClient.getSeconds();
}

void loop()
{
  timeClient.update();
  int current_seconds = timeClient.getSeconds();
  if (last_seconds != current_seconds)
  {
    last_seconds = current_seconds;
    status->update_status();
  }
}

void servePostForCurrentParams(AsyncWebServerRequest *request, JsonVariant &json)
{
  JsonObject jsonObj = json.as<JsonObject>();
  String json_str = "";
  serializeJsonPretty(jsonObj, json_str);
  Serial.println(json_str);
  bool isSuccess = status->setParameters(jsonObj);
  if (isSuccess)
  {
    status->saveParametersToFile();
    request->send(200, "text/json", "{\"success\":true}");
  }
  else
  {
    request->send(500, "text/json", "{\"success\":false,\"error_msg\":\"" + status->getErrorMessage() + "\"}");
  }
}