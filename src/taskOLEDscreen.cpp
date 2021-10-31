#include "./common.h"
#include "./params.h"
#include "SSD1306.h"   // OLED screen model library


// SSD OLED pin definitions

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    868E6 // LoRa MHz

// Create OLED instance "display"
SSD1306 display(0x3c, 21, 22);

void TaskOLEDscreen(void* pvParameters) {
  (void)pvParameters;
  // Initialize screen
  display.init();
  display.flipScreenVertically();  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  display.setFont(ArialMT_Plain_24);

   while (true){
	   if (false) {
  String p_temperature = String(getParameter(PARAM_TEMPERATURE)/100.0);
  String p_humidity = String(getParameter(PARAM_HUMIDITY)/100.0);
  String p_temperature2 = String(getParameter(PARAM_TEMPERATURE_EXT)/100.0);
  String p_temperature3 = String(getParameter(PARAM_TEMPERATURE_EXT2)/100.0);
  display.clear();
  display.drawString(0 , 0 , p_temperature + "°C");
  display.drawStringMaxWidth(0 , 20 , 128, p_humidity +"%");
  display.drawString(0 , 40 , p_temperature2 + "°C " + p_temperature3 + "°C");
	   }

  display.clear();
  display.drawString(0, 0,  String(getParameter(PARAM_WIFI_RSSI))+" dB"); 
  
  display.display();  
  vTaskDelay(1000);
  }
}

void taskOLEDscreen() {
  xTaskCreatePinnedToCore(TaskOLEDscreen, "TaskOLEDscreen",
                          2000,  // This stack size can be checked & adjusted by
                                 // reading the Stack Highwater
                          NULL,
                          2,  // Priority, with 3 (configMAX_PRIORITIES - 1)
                              // being the highest, and 0 being the lowest.
                          NULL, 1);
}
