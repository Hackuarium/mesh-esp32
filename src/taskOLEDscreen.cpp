#include "./common.h"
#include "./params.h"
#include "SSD1306.h"   // OLED screen model library
// Basic Config
#include <esp_spi_flash.h> // needed for reading ESP32 chip attributes


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

static int dp_row = 0, dp_col = 0, dp_font = 0;

void dp_println(int lines) {
  dp_col = 0;
  dp_row += lines;
#if (HAS_DISPLAY) == 1
  dp_setTextCursor(dp_col, dp_row);
#elif (HAS_DISPLAY) == 2
  for (int i = 1; i <= lines; i++)
    tft.println();
#endif
};

void dp_printf(const char *format, ...) {
  char loc_buf[64];
  char *temp = loc_buf;
  va_list arg;
  va_list copy;
  va_start(arg, format);
  va_copy(copy, arg);
  int len = vsnprintf(temp, sizeof(loc_buf), format, copy);
  va_end(copy);
  if (len < 0) {
    va_end(arg);
    return;
  };
  if (len >= sizeof(loc_buf)) {
    temp = (char *)malloc(len + 1);
    if (temp == NULL) {
      va_end(arg);
      return;
    }
    vsnprintf(temp, len + 1, format, arg);
  }
  va_end(arg);
#if (HAS_DISPLAY) == 1
  obdWriteString(&ssoled, 0, -1, dp_row, temp, dp_font >> 1, dp_font & 0x01,
                 false);
#elif (HAS_DISPLAY) == 2
  tft.printf(temp);
#endif
  if (temp != loc_buf) {
    free(temp);
  }
}

void TaskOLEDscreen(void* pvParameters) {
  (void)pvParameters;
  // Initialize screen
  display.init();
  display.flipScreenVertically();  
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  //display.setFont(ArialMT_Plain_24);

   while (true){
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    display.clear();
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

    display.drawString(0, 0,"** PAXCOUNTER **");
    display.drawString(0, 18,"ESP32 cores" + String(chip_info.cores));
    display.drawString(0, 36, "Chip Rev. " + String(chip_info.revision));
    String a = (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "";
    String b = (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "";
    display.drawString(0, 54, "WiFi" + a + b);
    String c = (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "int." : "ext.";
    display.drawString(0, 72, String(spi_flash_get_chip_size() / (1024 * 1024)) + "MB " + c + " Flash");
  //display.clear();
  //display.drawString(0, 0,  String(getParameter(PARAM_WIFI_RSSI))+" dB"); 
  
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
