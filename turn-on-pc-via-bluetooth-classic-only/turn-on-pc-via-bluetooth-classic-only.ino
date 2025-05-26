#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"

// Includes the configuration file (copy config.example.h to config.h and edit)
// If the config.h file doesn't exist, the compiler will show an error
#include "../config.h"

// ======== USER CONFIGURATION ========
// Settings are now in the config.h file

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ====== AUTHORIZED MAC LIST ======

// Authorized Bluetooth Classic MACs (lowercase)
const char* allowedClassicMacs[] = ALLOWED_CLASSIC_MACS;
const int numClassicMacs = sizeof(allowedClassicMacs) / sizeof(allowedClassicMacs[0]);

// PC MAC address for Wake-on-LAN
const uint8_t pcMacAddress[] = PC_MAC_ADDRESS;

IPAddress broadcastIP(BROADCAST_IP_1, BROADCAST_IP_2, BROADCAST_IP_3, BROADCAST_IP_4);
WiFiUDP udp;

unsigned long lastWakeSent = 0;
const unsigned long wakeCooldown = WAKE_COOLDOWN; // time between sends

void sendWakeOnLan(const uint8_t* mac) {
  uint8_t magicPacket[102];
  memset(magicPacket, 0xFF, 6);
  for (int i = 0; i < 16; ++i) {
    memcpy(&magicPacket[6 + i * 6], mac, 6);
  }
  udp.beginPacket(broadcastIP, WOL_PORT);
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();

  Serial.println("Wake-on-LAN packet sent!");
}

bool isAllowedMac(const char* mac, const char* list[], int listSize) {
  for (int i = 0; i < listSize; i++) {
    if (strcasecmp(mac, list[i]) == 0) {
      return true;
    }
  }
  return false;
}

// -------------------- BLUETOOTH CLASSIC CALLBACK --------------------

void btGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  if (event == ESP_BT_GAP_DISC_RES_EVT) {
    char bda_str[18];
    sprintf(bda_str, "%02x:%02x:%02x:%02x:%02x:%02x",
            param->disc_res.bda[0], param->disc_res.bda[1], param->disc_res.bda[2],
            param->disc_res.bda[3], param->disc_res.bda[4], param->disc_res.bda[5]);
    // Serial.printf("[Classic] Device found: %s\n", bda_str);

    if (isAllowedMac(bda_str, allowedClassicMacs, numClassicMacs)) {
      if (millis() - lastWakeSent > wakeCooldown) {
        Serial.println("[Classic] Authorized device detected! Sending Wake-on-LAN...");
        sendWakeOnLan(pcMacAddress);
        lastWakeSent = millis();
      } else {
        Serial.println("[Classic] Authorized device detected! but it is in cooldown...");
      }
    }
  }
  else if (event == ESP_BT_GAP_DISC_STATE_CHANGED_EVT) {
    if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
      Serial.println("[Classic] Scan stopped.");
    } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) {
      Serial.println("[Classic] Scan started.");
    }
  }
}

// --------------------- SETUP ---------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Initialize UDP (using the configured WoL port)
  udp.begin(WOL_PORT);

  // Initialize Bluetooth in dual mode (BLE + Classic)
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t ret;

  ret = esp_bt_controller_init(&bt_cfg);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Error initializing BT controller: %s\n", esp_err_to_name(ret));
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Error enabling BT controller: %s\n", esp_err_to_name(ret));
    return;
  }

  ret = esp_bluedroid_init();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Error initializing bluedroid: %s\n", esp_err_to_name(ret));
    return;
  }

  ret = esp_bluedroid_enable();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Error enabling bluedroid: %s\n", esp_err_to_name(ret));
    return;
  }

  Serial.println("System initialized: Wi-Fi, Bluetooth Classic ready.");
}

// -------------------- LOOP --------------------

void loop() {
  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 1, 0);
  delay(2000);
}
