#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"

// Inclui o arquivo de configuração (copie config.example.h para config.h e edite)
// Se o arquivo config.h não existir, o compilador mostrará um erro
#include "../config.h"

// ======== CONFIGURAÇÃO DO USUÁRIO ========
// As configurações agora estão no arquivo config.h

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// ====== LISTA DE MAC AUTORIZADOS ======

// BLE MACs (em minúsculas, sem maiúsculas!)
const char* allowedBLEMacs[] = ALLOWED_BLE_MACS;
const int numBLEMacs = sizeof(allowedBLEMacs) / sizeof(allowedBLEMacs[0]);

// Bluetooth Classic MACs autorizados (minúsculas)
const char* allowedClassicMacs[] = ALLOWED_CLASSIC_MACS;
const int numClassicMacs = sizeof(allowedClassicMacs) / sizeof(allowedClassicMacs[0]);

// MAC do PC para Wake-on-LAN
const uint8_t pcMacAddress[] = PC_MAC_ADDRESS;

IPAddress broadcastIP(BROADCAST_IP_1, BROADCAST_IP_2, BROADCAST_IP_3, BROADCAST_IP_4);
WiFiUDP udp;

BLEScan* pBLEScan;

const int bleScanTime = BLE_SCAN_INTERVAL;    // segundos scan BLE
const int classicScanTime = CLASSIC_SCAN_CYCLES; // ciclos do scan clássico (cada ciclo 1.28s)

unsigned long lastWakeSent = 0;
const unsigned long wakeCooldown = WAKE_COOLDOWN; // tempo entre envios

bool scanningBLE = true;

void sendWakeOnLan(const uint8_t* mac) {
  uint8_t magicPacket[102];
  memset(magicPacket, 0xFF, 6);
  for (int i = 0; i < 16; ++i) {
    memcpy(&magicPacket[6 + i * 6], mac, 6);
  }
  udp.beginPacket(broadcastIP, 9); // Porta padrão para Wake-on-LAN é 9, mas algumas implementações usam 7
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();

  Serial.println("Wake-on-LAN packet enviado!");
}

bool isAllowedMac(const char* mac, const char* list[], int listSize) {
  for (int i = 0; i < listSize; i++) {
    if (strcasecmp(mac, list[i]) == 0) {
      return true;
    }
  }
  return false;
}

// -------------------- CALLBACK BLUETOOTH CLASSIC --------------------

void btGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  if (event == ESP_BT_GAP_DISC_RES_EVT) {
    char bda_str[18];
    sprintf(bda_str, "%02x:%02x:%02x:%02x:%02x:%02x",
            param->disc_res.bda[0], param->disc_res.bda[1], param->disc_res.bda[2],
            param->disc_res.bda[3], param->disc_res.bda[4], param->disc_res.bda[5]);
    Serial.printf("[Classic] Device found: %s\n", bda_str);

    if (isAllowedMac(bda_str, allowedClassicMacs, numClassicMacs)) {
      if (millis() - lastWakeSent > wakeCooldown) {
        Serial.println("[Classic] Dispositivo autorizado detectado! Enviando Wake-on-LAN...");
        sendWakeOnLan(pcMacAddress);
        lastWakeSent = millis();
      }
    }
  }
  else if (event == ESP_BT_GAP_DISC_STATE_CHANGED_EVT) {
    if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
      Serial.println("[Classic] Scan parado.");
    } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) {
      Serial.println("[Classic] Scan iniciado.");
    }
  }
}

// --------------------- SETUP ---------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Conectar ao Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Inicializar UDP (porta 9 padrão WoL)
  udp.begin(9);

  // Inicializar Bluetooth no modo dual (BLE + Classic)
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t ret;

  ret = esp_bt_controller_init(&bt_cfg);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao inicializar controlador BT: %s\n", esp_err_to_name(ret));
    return;
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao habilitar controlador BT: %s\n", esp_err_to_name(ret));
    return;
  }

  ret = esp_bluedroid_init();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao inicializar bluedroid: %s\n", esp_err_to_name(ret));
    return;
  }

  ret = esp_bluedroid_enable();
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao habilitar bluedroid: %s\n", esp_err_to_name(ret));
    return;
  }

  // Iniciar BLE
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);  // escaneamento ativo para resultados mais rápidos

  // Iniciar Bluetooth Classic
  esp_bt_gap_register_callback(btGapCallback);
  esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, classicScanTime, 0);

  Serial.println("Sistema inicializado: Wi-Fi, BLE e Bluetooth Classic prontos.");
}

// -------------------- LOOP --------------------

void loop() {
  static unsigned long lastBleScan = 0;
  static bool bleScanning = false;

  // Controle simples pra fazer scans alternados
  if (scanningBLE && !bleScanning && (millis() - lastBleScan > WAKE_COOLDOWN)) {
    Serial.println("[BLE] Iniciando scan BLE...");
    BLEScanResults* results = pBLEScan->start(bleScanTime, false);
    bleScanning = true;

    // Processa dispositivos BLE encontrados
    for (int i = 0; i < results->getCount(); i++) {
      BLEAdvertisedDevice d = results->getDevice(i);
      std::string macStr = std::string(d.getAddress().toString().c_str());
      std::string macLower;
      macLower.resize(macStr.size());
      std::transform(macStr.begin(), macStr.end(), macLower.begin(), ::tolower);
      const char* macCstr = macLower.c_str();

      std::string nameStr = std::string(d.getName().c_str());
      Serial.printf("[BLE] Encontrado: %s", macCstr);
      if (!nameStr.empty()) {
        Serial.printf(" (%s)", nameStr.c_str());
      }
      Serial.println();

      if (isAllowedMac(macCstr, allowedBLEMacs, numBLEMacs)) {
        if (millis() - lastWakeSent > wakeCooldown) {
          Serial.println("[BLE] Dispositivo autorizado! Enviando Wake-on-LAN...");
          sendWakeOnLan(pcMacAddress);
          lastWakeSent = millis();
        }
      }
    }

    pBLEScan->clearResults();
    lastBleScan = millis();
    bleScanning = false;

    // Após scan BLE, troca para scan Classic
    scanningBLE = false;
    // Reinicia scan Classic (se não estiver rodando)
    esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, classicScanTime, 0);
  }

  // O scan Bluetooth Classic roda via callback, só alternamos entre BLE e Classic scans para não travar o dispositivo
  if (!scanningBLE) {
    // Espera terminar scan Classic antes de voltar para BLE
    // ESP_BT_GAP_DISC_STATE_CHANGED_EVT é chamado ao parar scan Classic (mas não usamos aqui)
    // Aí só volta para scan BLE após 30s (controlado acima)
    // Por simplicidade, aqui só alterna após cooldown
    if (millis() - lastBleScan > 4000) {
      scanningBLE = true;
    }
  }
}