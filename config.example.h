// config.example.h - Example configuration file for the ESP32 Turn On PC via Bluetooth project
// Copy this file to config.h and edit with your settings

#ifndef CONFIG_H
#define CONFIG_H

// ======== WI-FI SETTINGS ========
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// ======== NETWORK SETTINGS ========
// Local network broadcast address (replace with yours)
#define BROADCAST_IP_1 192
#define BROADCAST_IP_2 168
#define BROADCAST_IP_3 0
#define BROADCAST_IP_4 255

// ======== TARGET PC SETTINGS ========
// PC MAC address for Wake-on-LAN (format: 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX)
#define PC_MAC_ADDRESS { 0x3C, 0x52, 0x82, 0xAB, 0xCD, 0xEF }

// ======== AUTHORIZED BLUETOOTH DEVICES ========
// List of authorized BLE MACs (in lowercase)
#define ALLOWED_BLE_MACS { \
  "xx:xx:xx:xx:xx:xx", /* Device 1 name */ \
  "yy:yy:yy:yy:yy:yy"  /* Device 2 name */ \
}

// List of authorized Bluetooth Classic MACs (in lowercase)
#define ALLOWED_CLASSIC_MACS { \
  "xx:xx:xx:xx:xx:xx", /* Device 1 name */ \
  "yy:yy:yy:yy:yy:yy"  /* Device 2 name */ \
}

// ======== TIMING SETTINGS ========
// BLE scanning interval (in seconds)
#define BLE_SCAN_INTERVAL 1

// Bluetooth Classic scanning cycles (each cycle ~1.28s)
#define CLASSIC_SCAN_CYCLES 5

// Wait time between Wake-on-LAN sends (in milliseconds)
#define WAKE_COOLDOWN 30000

// Time to switch between BLE and Classic scans (in milliseconds)
#define CLASSIC_BLE_SWITCH_TIME 4000

// ======== NETWORK SETTINGS ========
// Wake-on-LAN UDP port (default is 9, some implementations use 7)
#define WOL_PORT 9

#define LED_PIN 2  // Pino do LED onboard

#endif // CONFIG_H
