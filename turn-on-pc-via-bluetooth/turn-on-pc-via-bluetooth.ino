#include <WiFi.h>
#include <esp_wifi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <WiFiUdp.h>

// Includes the configuration file (copy config.example.h to config.h and edit)
// If the config.h file doesn't exist, the compiler will show an error
#include "../config.h"

// ======== USER CONFIGURATION ========
// Settings are now in the config.h file

enum LedStatus {
  OFF,
  GREEN,
  YELLOW,
  RED
};

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

// ======== END OF USER CONFIGURATION ========

BLEScan* pBLEScan;

// Function to send a Wake-on-LAN magic packet
void sendWakeOnLan(const uint8_t* mac) {
  WiFiUDP udp;
  uint8_t magicPacket[102];

  // Build the magic packet
  memset(magicPacket, 0xFF, 6);
  for (int i = 0; i < 16; ++i) {
    memcpy(&magicPacket[6 + i * 6], mac, 6);
  }

  udp.beginPacket(broadcastIP, WOL_PORT);
  udp.write(magicPacket, sizeof(magicPacket));
  udp.endPacket();

  Serial.println("Wake-on-LAN packet sent to PC!");
}

void setLedStatus(LedStatus status) {
  switch (status) {
    case OFF:
      digitalWrite(LED_PIN, LOW);
      break;
    case GREEN:
      digitalWrite(LED_PIN, HIGH); // LED acende (verde simbólico)
      break;
    case YELLOW:
      {
        unsigned long cooldownStart = millis();
        while (millis() - cooldownStart < WAKE_COOLDOWN) {
          digitalWrite(LED_PIN, HIGH);
          delay(300);
          digitalWrite(LED_PIN, LOW);
          delay(300);
        }
      }
      break;
    case RED:
      for (int i = 0; i < 6; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
      break;
  }
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

  pinMode(LED_PIN, OUTPUT);
  setLedStatus(OFF);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to Wi-Fi...");
  int wifi_attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    wifi_attempts++;
    if (wifi_attempts > 20) {
      Serial.println("\nFailed to connect to Wi-Fi!");
      setLedStatus(RED);
      return;
    }
  }
  Serial.println("\nWi-Fi connected!");

  // Initialize BLE scanning
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true); // Active scan for faster results
}

void loop() {
  setLedStatus(OFF);
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
      setLedStatus(GREEN);
      sendWakeOnLan(pcMacAddress);
      setLedStatus(YELLOW); // Wait before scanning again to avoid repeat triggers
      break;
    }
  }

  pBLEScan->clearResults(); // Free memory
}