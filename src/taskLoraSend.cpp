#include "./common.h"
#include "./params.h"
#include <SPI.h>
#include <LoRa.h>

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISnO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND  868E6

void TaskLora(void* pvParameters) {

  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  while (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    vTaskDelay(5000);
  }


  while (true) {
    LoRa.beginPacket();
    printCompactParameters(&LoRa,8);
    LoRa.endPacket();

    vTaskDelay(10 * 1000);
  }
}


void taskLora() {
  xTaskCreatePinnedToCore(TaskLora, "TaskLora",
                          16000,  // This stack size can be checked & adjusted
                                  // by reading the Stack Highwater
                          NULL,
                          3,  // Priority, with 3 (configMAX_PRIORITIES - 1)
                              // being the highest, and 0 being the lowest.
                          NULL, 1);
}



