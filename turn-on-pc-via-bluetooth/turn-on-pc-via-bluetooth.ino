#include <WiFi.h>
#include <esp_wifi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <WiFiUdp.h>

// Inclui o arquivo de configuração (copie config.example.h para config.h e edite)
// Se o arquivo config.h não existir, o compilador mostrará um erro
#include "../config.h"

// ======== CONFIGURAÇÃO DO USUÁRIO ========
// As configurações agora estão no arquivo config.h

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// List of allowed Bluetooth MACs that can trigger Wake-on-LAN
const char* allowedBLEMacs[] = ALLOWED_BLE_MACS;
const int numBLEMacs = sizeof(allowedBLEMacs) / sizeof(allowedBLEMacs[0]);

// MAC address of the PC to be woken up (target of Wake-on-LAN)
const uint8_t pcMacAddress[] = PC_MAC_ADDRESS;

// Local network broadcast IP address
IPAddress broadcastIP(BROADCAST_IP_1, BROADCAST_IP_2, BROADCAST_IP_3, BROADCAST_IP_4);

// BLE scan interval (in seconds)
const int scanInterval = BLE_SCAN_INTERVAL;

// ======== FIM DA CONFIGURAÇÃO DO USUÁRIO ========

BLEScan* pBLEScan;

// Function to send a Wake-on-LAN magic packet
void sendWakeOnLan(const uint8_t* mac) {
  WiFiUDP udp;
  const int port = 9; // Porta padrão para Wake-on-LAN é 9, mas algumas implementações usam 7
  uint8_t magicPacket[102];

  // Build the magic packet
  memset(magicPacket, 0xFF, 6);
  for (int i = 0; i < 16; ++i) {
    memcpy(&magicPacket[6 + i * 6], mac, 6);
  }

  udp.beginPacket(broadcastIP, port);
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();

  Serial.println("Wake-on-LAN packet sent to PC!");
}

// Check if the scanned BLE MAC is in the allowed list
bool isAllowedBLE(const std::string& mac) {
  for (int i = 0; i < numBLEMacs; ++i) {
    if (mac == allowedBLEMacs[i]) {
      return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");

  // Initialize BLE scanning
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true); // Active scan for faster results
}

void loop() {
  Serial.println("Scanning for BLE devices...");

  // get pointer instead of object
  BLEScanResults* results = pBLEScan->start(scanInterval, false);

  for (int i = 0; i < results->getCount(); i++) {
    BLEAdvertisedDevice d = results->getDevice(i);

    // Convert Arduino String to std::string
    std::string mac = std::string(d.getAddress().toString().c_str());
    std::string name = std::string(d.getName().c_str());

    // Serial.print("Found device: ");
    // Serial.print(mac.c_str());
    // if (!name.empty()) {
    //   Serial.print(" (");
    //   Serial.print(name.c_str());
    //   Serial.print(")");
    // }
    // Serial.println();

    if (isAllowedBLE(mac)) {
      Serial.println("Authorized BLE device detected. Sending Wake-on-LAN...");
      sendWakeOnLan(pcMacAddress);
      delay(WAKE_COOLDOWN);  // Wait before scanning again to avoid repeat triggers
      break;
    }
  }

  pBLEScan->clearResults(); // Free memory
}