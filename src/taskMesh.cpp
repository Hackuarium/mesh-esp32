#include <AsyncMqttClient.h>
#include <WiFiClient.h>
#include <painlessMesh.h>
#include "./params.h"

#ifdef MESH
#ifndef PARAM_MESH_CLIENT

#define MESH_PREFIX "mesh"
#define MESH_PASSWORD "password"
#define MESH_PORT 5555

#define HOSTNAME "MQTT_Bridge"

// Prototypes
void meshReceivedCallback(const uint32_t& from, const String& msg);
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0, 0, 0, 0);

painlessMesh mesh;
WiFiClient wifiClient;

char ssid[20];
char password[20];
char broker[20];
char topic[20];

AsyncMqttClient mqttMeshClient;

void onMqttPublish(uint16_t packetId);
void onMqttSubscribe(uint16_t packetId, uint8_t qos);
void onMqttUnsubscribe(uint16_t packetId);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttMessage(char* topic,
                   char* payload,
                   AsyncMqttClientMessageProperties properties,
                   size_t len,
                   size_t index,
                   size_t total);

void TaskMesh(void* pvParameters) {
  getParameter("wifi.ssid", ssid);
  getParameter("wifi.password", password);
  getParameter("mqtt.broker", broker);
  getParameter("mqtt.topic", topic);

  mesh.setDebugMsgTypes(
      ERROR | STARTUP |
      CONNECTION);  // set before init() so that you can see startup messages

  // Channel set to 6. Make sure to use the same channel for your mesh and for
  // you other network (STATION_SSID)
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 6);
  mesh.onReceive(&meshReceivedCallback);

  mesh.stationManual(ssid, password);
  mesh.setHostname(HOSTNAME);

  // Bridge node, should (in most cases) be a root node. See [the
  // wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation)
  // for some background
  mesh.setRoot(false);
  // This node and all other nodes should ideally know the mesh contains a root,
  // so call this on all nodes
  mesh.setContainsRoot(true);

  mqttMeshClient.setServer(broker, 1883);
  mqttMeshClient.connect();
  mqttMeshClient.onConnect(onMqttConnect);
  mqttMeshClient.onPublish(onMqttPublish);

  while (true) {
    mesh.update();
    auto nodes = mesh.getNodeList(true);
    String str;
    for (auto&& id : nodes)
      str += String(id) + String(" ");
    mqttMeshClient.publish("painlessMesh/from/gateway", 0, true, str.c_str());
    static int waitA = millis();
    static uint16_t i = 0;
    if (i > 6000) {
      Serial.println("Node list:" + String(str.c_str()));
      i = 0;
    }
    if (waitA != millis()) {
      i++;
    }

    if (myIP != getlocalIP()) {
      myIP = getlocalIP();
      Serial.println("My IP is " + myIP.toString());
    }

    uint16_t packetIdPub1 =
        mqttMeshClient.publish("test/esp", 0, true, "Hello 123");
    vTaskDelay(1000);
  }
}

void taskMesh() {
  // Now set up two tasks to run independently.
  xTaskCreatePinnedToCore(TaskMesh, "TaskMesh",
                          8000,  // This stack size can be checked & adjusted by
                                 // reading the Stack Highwater
                          NULL,
                          2,  // Priority, with 3 (configMAX_PRIORITIES - 1)
                              // being the highest, and 0 being the lowest.
                          NULL, 1);
}

void meshReceivedCallback(const uint32_t& from, const String& msg) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "painlessMesh/from/" + String(from);
  mqttMeshClient.publish(topic.c_str(), 0, true, msg.c_str());
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length + 1);
  memcpy(cleanPayload, payload, length);
  cleanPayload[length] = '\0';
  String msg = String(cleanPayload);
  free(cleanPayload);

  String targetStr = String(topic).substring(16);

  if (targetStr == "gateway") {
    if (msg == "getNodes") {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto&& id : nodes)
        str += String(id) + String(" ");
      mqttMeshClient.publish("painlessMesh/from/gateway", 0, true, str.c_str());
    }
  } else if (targetStr == "broadcast") {
    mesh.sendBroadcast(msg);
  } else {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if (mesh.isConnected(target)) {
      mesh.sendSingle(target, msg);
    } else {
      mqttMeshClient.publish("painlessMesh/from/gateway", 0, true,
                             "Client not connected!");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  // uint16_t packetIdSub = mqttClient.subscribe(topic, 2);
  // Serial.print("Subscribing at QoS 2, packetId: ");
  // Serial.println(packetIdSub);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

#endif
#endif