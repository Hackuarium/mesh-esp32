

#include "./common.h"
#include "./params.h"
#include "taskNTPD.h"

void taskBlink();
void taskSerial();
void taskWebserver();
void taskNTPD();
void taskOTA();
void taskLora();
void taskMDNS();
void taskWifi();
// void taskMQTT();
#ifdef PARAM_MESH_CLIENT
void taskMeshClient();
#else
void taskMesh();
#endif

void taskSI7021();
// void taskOneWire();
void taskOLEDscreen();

void setup() {
  Serial.begin(115200);  // only for debug purpose
  setupParameters();
  Serial.println("Setup Serial:");
  taskSerial();
  // taskWebserver();
  // taskNTPD();
  // taskLora();
  // taskSI7021();
  // taskOneWire();
  taskOLEDscreen();
  // taskOTA();
  //  taskMDNS();
  // taskWifi();
  // taskMQTT();
#ifdef PARAM_MESH_CLIENT
  taskMeshClient();
#elif
  taskMesh();
#endif
  taskBlink();
}

void loop() {
  vTaskDelay(10000);
}