#include "./common.h"
#include "./params.h"
#include "SSD1306.h"  // OLED screen model library
// Basic Config
#include <esp_spi_flash.h>  // needed for reading ESP32 chip attributes

// SSD OLED pin definitions

#define SCK 5       // GPIO5  -- SX1278's SCK
#define MISO 19     // GPIO19 -- SX1278's MISO
#define MOSI 27     // GPIO27 -- SX1278's MOSI
#define SS 18       // GPIO18 -- SX1278's CS
#define RST 14      // GPIO14 -- SX1278's RESET
#define DI0 26      // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND 868E6  // LoRa MHz

// Create OLED instance "display"
SSD1306 display(0x3c, 21, 22);

static int dp_row = 0, dp_col = 0;

void dp_drawPage() {
  // write display content to display buffer
  // nextpage = true -> flip 1 page

  static uint8_t DisplayPage = 0;
  char timeState;

  // cursor home
  display.clear();

  // line 1/2: pax counter
  // display number of unique macs total Wifi + BLE
  // ---------- page 0: parameters overview ----------

  // line 3: wifi + bluetooth counters
  // WIFI:abcde BLTH:abcde

#if ((WIFICOUNTER) && (BLECOUNTER))
  if (cfg.wifiscan)
    dp_printf("WIFI:%-5d", count_from_libpax.wifi_count);
  else
    dp_printf("WIFI:off");
  if (cfg.blescan)
#if (COUNT_ENS)
    if (cfg.enscount)
      dp_printf(" CWA:%-5d", cwa_report());
    else
#endif
      dp_printf("BLTH:%-5d", count_from_libpax.ble_count);
  else
    dp_printf(" BLTH:off");
#elif ((WIFICOUNTER) && (!BLECOUNTER))
  if (cfg.wifiscan)
    dp_printf("WIFI:%-5d", count_from_libpax.wifi_count);
  else
    dp_printf("WIFI:off");
#elif ((!WIFICOUNTER) && (BLECOUNTER))
  if (cfg.blescan)
    dp_printf("BLTH:%-5d", count_from_libpax.ble_count);
#if (COUNT_ENS)
  if (cfg.enscount)
    dp_printf("(CWA:%d)", cwa_report());
  else
#endif
    dp_printf("BLTH:off");
#else
  // dp_printf("Sniffer disabled");
#endif
    // dp_println();

    // line 4: Battery + GPS status + Wifi channel
    // B:a.bcV Sats:ab ch:ab
#if (defined BAT_MEASURE_ADC || defined HAS_PMU || defined HAS_IP5306)
  if (batt_level == 0)
    dp_printf("No batt ");
  else
    dp_printf("B:%3d%%  ", batt_level);
#else
    // dp_printf("       ");
#endif

  // line 5: RSSI limiter + free memory
  // RLIM:abcd  Mem:abcdKB
  display.clear();
  // line 7: LMIC status
  // yyyyyyyyyyyyy xx SFab

#if (HAS_LORA)
  // LMiC event display
  dp_printf("%-16s ", lmic_event_msg);
  // LORA datarate, display inverse if ADR disabled
  dp_setFont(MY_FONT_SMALL, !cfg.adrmode);
  dp_printf("%-4s", getSfName(updr2rps(LMIC.datarate)));
  dp_setFont(MY_FONT_SMALL, 0);
#endif  // HAS_LORA

}  // dp_drawPage

void TaskOLEDscreen(void* pvParameters) {
  (void)pvParameters;
  // Initialize screen
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);
  // display.setFont(ArialMT_Plain_24);

  while (true) {
    display.clear();
    switch (0) {
      case 0: {
        display.drawString(
            0, 0, "Node ID: " + String(getParameter(PARAM_MESH_NODE_ID)));
        display.drawString(
            0, 20, "Nb nodes: " + String(getParameter(PARAM_MESH_NB_NODES)));
        break;
      }
      case 1: {
        display.drawString(
            0, 0, String(getParameter(PARAM_TEMPERATURE) / 100.0) + "°C");
        display.drawStringMaxWidth(
            0, 20, 128, String(getParameter(PARAM_HUMIDITY) / 100.0) + "%");
        display.drawString(
            0, 40,
            String(getParameter(PARAM_TEMPERATURE_EXT) / 100.0) + "°C " +
                String(getParameter(PARAM_TEMPERATURE_EXT2) / 100.0) + "°C");
        break;
      }
      case 2: {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        display.drawString(0, 0, "ESP32 cores " + String(chip_info.cores));
        // display.drawString(0, 18, "Chip Rev. " + String(chip_info.revision));
        String a = (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "";
        String b = (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "";
        display.drawString(0, 16, "WiFi" + a + b);
        String c =
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "int." : "ext.";
        display.drawString(0, 34,
                           String(spi_flash_get_chip_size() / (1024 * 1024)) +
                               "MB " + c + " Flash");
        break;
      }
    }
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
