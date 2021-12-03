#include <Arduino.h>

// #define THR_WIRE_MASTER 1

#define LED_BUILTIN 25

#define MAX_PARAM 78
extern int16_t parameters[MAX_PARAM];

#define MAX_LED 256

#define PARAM_ERROR 51

#define PARAM_TEMPERATURE 0  // A
#define PARAM_HUMIDITY 1
#define PARAM_TEMPERATURE_EXT 2
#define PARAM_TEMPERATURE_EXT2 3

#define PARAM_WIFI_RSSI 49  // AX  Wifi RSSI, larger is better

#define MAX_NUMBER_OF_CONNECTED 25

#define PARAM_MESH_NB_NODES 52
#define PARAM_MESH_NODE_ID 53  // BA
#define PARAM_MESH_CONN_1 54
#define PARAM_MESH_CONN_2 55
// ...
#define PARAM_MESH_CONN_25 78

#define MESH 1
// Define MQTT Client or Bridge
//#define PARAM_MESH_CLIENT 1