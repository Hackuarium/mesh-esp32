#include "./common.h"
#include "./params.h"

#include <painlessMesh.h>

#define MESH_SSID "mesh"
#define MESH_PASSWORD "password"
#define MESH_PORT 5555

// Prototypes
void sendMessage();
void receivedCallback(uint32_t from, String& msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler userScheduler;  // to control your personal task
painlessMesh mesh;

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void sendMessage();  // Prototype
Task taskSendMessage(TASK_SECOND * 1,
                     TASK_FOREVER,
                     &sendMessage);  // start with a one second interval

// Task to blink the number of nodes
bool onFlag = false;

void sendMessage() {
  String msg = "Hello from node ";
  msg += mesh.getNodeId();
  msg += " myFreeMemory: " + String(ESP.getFreeHeap());
  mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  Serial.printf("Sending message: %s\n", msg.c_str());

  taskSendMessage.setInterval(
      random(TASK_SECOND * 1, TASK_SECOND * 5));  // between 1 and 5 seconds
}

void receivedCallback(uint32_t from, String& msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> startHere: New Connection, %s\n",
                mesh.subConnectionJson(true).c_str());
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  nodes = mesh.getNodeList();

  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}

void TaskMeshClient(void* pvParameters) {
  mesh.setDebugMsgTypes(
      ERROR | DEBUG);  // set before init() so that you can see error messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  while (true) {
    mesh.update();
    setParameter(PARAM_MESH_NODE_ID, mesh.getNodeId() & 0xFF);
    setParameter(PARAM_MESH_NB_NODES, nodes.size() + 1);
    vTaskDelay(1000);
  }
}

void taskMeshClient() {
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(TaskMeshClient, "TaskMesh",
                          8000,  // This stack size can be checked & adjusted by
                                 // reading the Stack Highwater
                          NULL,
                          2,  // Priority, with 3 (configMAX_PRIORITIES - 1)
                              // being the highest, and 0 being the lowest.
                          NULL, 1);
}