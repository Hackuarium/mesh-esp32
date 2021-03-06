#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <StringStream.h> 
#include <WiFi.h>
#include "./SerialUtilities.h"
#include "./common.h"
#include "./taskSerial.cpp"

AsyncWebServer server(80);

void notFound(AsyncWebServerRequest* request) {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";
  for (uint8_t i = 0; i < request->args(); i++) {
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }
  request->send(404, "text/plain", message);
}

void handleCommand(AsyncWebServerRequest* request) {
  String action = request->arg("value");
  if (action == "") {
    action = "h";
  }
  String message;
  StringStream stream((String&)message);
  printResult(&action[0], &stream);

  request->send(200, "text/plain", message);
}

void TaskWebserver(void* pvParameters) {
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(5000);
  }

  (void)pvParameters;

  server.on("/command", handleCommand);

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  server.serveStatic("/test.jpg", SPIFFS, "/test.jpg");

  server.onNotFound(notFound);

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  Serial.println("HTTP server started");

  while (true) {
    vTaskDelay(1);
  }
}

void taskWebserver() {
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(TaskWebserver, "TaskWebserver",
                          20000,  // This stack size can be checked & adjusted
                                  // by reading the Stack Highwater
                          NULL,
                          3,  // Priority, with 3 (configMAX_PRIORITIES - 1)
                              // being the highest, and 0 being the lowest.
                          NULL, 1);
}
