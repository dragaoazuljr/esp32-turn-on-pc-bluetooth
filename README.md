# ESP32 Turn On PC via Bluetooth

This project uses an ESP32 microcontroller to automatically send a Wake-on-LAN (WoL) packet to turn on a PC when specific Bluetooth devices are detected nearby. It's perfect for automatically powering on your PC when you turn on a game controller or other Bluetooth device.

## Project Overview

This project allows you to automatically turn on your PC using an ESP32 whenever specific Bluetooth devices come online nearby — perfect for powering your PC with a controller.

The repository provides **four implementations**, each tailored to different Bluetooth protocols and hardware:

1. **BLE Only Implementation** (`turn-on-pc-via-bluetooth.ino`)

   * Uses **Bluetooth Low Energy (BLE)** scanning exclusively.
   * Suitable for **modern controllers** that broadcast BLE advertisements even after pairing (e.g., *8bitdo Ultimate 2 Bluetooth mode*).
   * Offers **faster response** and **lower power usage**.

2. **Dual Mode Implementation** (`turn-on-pc-via-bluetooth-ble-classic.ino`)

   * Alternates between **BLE and Bluetooth Classic** scanning.
   * Designed for **wider compatibility**, including **older devices** that don't support BLE.
   * Can detect devices like **DualShock 4** or **8bitdo SN30 Pro**, **but only when they are in pairing mode** (i.e., discoverable).

3. **Bluetooth Classic Only Implementation** (`turn-on-pc-via-bluetooth-classic-only.ino`)

   * Uses **Bluetooth Classic** scanning exclusively.
   * Optimized for **older devices** that only support Bluetooth Classic.
   * Continuously scans for devices in pairing/discovery mode.
   * Ideal for users who only need to detect Bluetooth Classic devices.

4. **Raspberry Pi Implementation** (`turn-on-pc-via-bluetooth-raspberry-pi/`)

   * Uses **Raspberry Pi** with Linux for enhanced Bluetooth detection.
   * Implements **multiple detection methods**:
     - `hcitool inq` for Bluetooth Classic devices
     - `hcidump` for monitoring Bluetooth traffic
     - `hcitool lescan` for BLE devices
   * **Best solution for Bluetooth Classic devices** that don't work with ESP32 outside discovery mode.
   * Can detect devices like **DualShock 4** even when not in pairing mode.
   * More robust and flexible, but requires more power and setup.

---

## Understanding BLE vs Bluetooth Classic

Some Bluetooth devices support **BLE** and will continuously broadcast their presence even after being paired with another device. These devices can be detected **immediately upon powering on**, making the BLE-only script ideal.

However, **many older controllers** only support **Bluetooth Classic**, and will only show up during a **short window while they are in pairing mode**. This means the ESP32 can't detect them unless the device is explicitly put into pairing mode.

### ✅ Devices that work well with BLE:

* **8bitdo Ultimate 2 (BLE mode)**
* Most modern fitness trackers, keyboards, and low-energy peripherals
  ➡ Detected even after pairing — ideal for the **BLE Only** script.

### ⚠️ Devices that require Bluetooth Classic:

* **DualShock 4**
* **8bitdo SN30 Pro**, older 8bitdo models
  ➡ Only visible during **pairing mode** — use the **Dual Mode** or **Classic Only** script.

> **Important Note**: When using the ESP32 implementation, Bluetooth Classic devices can only be detected when they are in pairing mode (pairable). However, the Raspberry Pi implementation can detect these devices both when they are in pairing mode AND when they are normally powered on. This makes the Raspberry Pi solution more convenient for daily use with controllers like the DualShock 4.

---

### 🔍 How to know if your device supports BLE?

You can test using an Android phone:

1. Open the **Bluetooth menu**.
2. Power on your device (controller, etc.).
3. If the device **shows up in the list while already paired or after turning on**, it likely supports BLE.
4. If it **only appears while in pairing mode**, it's probably Bluetooth Classic only.

BLE devices often advertise their presence as connectable, even if already connected to another device.

## Hardware Requirements

### ESP32 Implementation
- ESP32 development board (ESP32-WROOM, ESP32-WROVER, or similar)
- Power supply for the ESP32 (USB or external)
- PC with Wake-on-LAN capability enabled in BIOS/UEFI

### Raspberry Pi Implementation
- Raspberry Pi Zero 2 W (recommended) or any other Raspberry Pi model with Bluetooth
- Power supply for the Raspberry Pi
- PC with Wake-on-LAN capability enabled in BIOS/UEFI

## Software Requirements

### ESP32 Implementation
- Arduino IDE
- ESP32 board support package for Arduino
- Required libraries:
  - WiFi.h
  - WiFiUdp.h
  - BLEDevice.h (for BLE and dual mode implementations)
  - BLEUtils.h (for BLE and dual mode implementations)
  - BLEScan.h (for BLE and dual mode implementations)
  - esp_bt.h (for dual mode and Classic only implementations)
  - esp_bt_main.h (for dual mode and Classic only implementations)
  - esp_gap_bt_api.h (for dual mode and Classic only implementations)

### Raspberry Pi Implementation
- Raspberry Pi OS (or any Linux distribution)
- Required packages:
  - bluez (for Bluetooth tools)
  - wakeonlan (for WoL packets)

## Implementation Details

### BLE Only Implementation (`turn-on-pc-via-bluetooth.ino`)

This implementation scans only for BLE devices. It's simpler and uses less power but may not detect older Bluetooth Classic devices.

Key features:
- Scans for BLE devices at regular intervals
- Checks detected devices against an authorized list
- Sends Wake-on-LAN packet when authorized device is found
- Implements a cooldown period to prevent multiple wake signals

### Dual Mode Implementation (`turn-on-pc-via-bluetooth-ble-classic.ino`)

This implementation scans for both BLE and Bluetooth Classic devices, providing wider compatibility with various controllers and devices.

Key features:
- Alternates between BLE and Bluetooth Classic scanning
- Maintains separate authorized device lists for each protocol
- Uses callbacks for Bluetooth Classic device detection
- Implements a cooldown period to prevent multiple wake signals
- Provides more detailed logging

### Bluetooth Classic Only Implementation (`turn-on-pc-via-bluetooth-classic-only.ino`)

This implementation focuses exclusively on Bluetooth Classic devices, optimized for older controllers and peripherals.

Key features:
- Uses only Bluetooth Classic scanning (no BLE)
- Continuously restarts scanning when completed
- Attempts to retrieve device names when available
- Implements automatic Wi-Fi reconnection
- Provides detailed logging of detected devices

### Raspberry Pi Implementation (`turn-on-pc-via-bluetooth-raspberry-pi/`)

This implementation uses a Raspberry Pi to provide enhanced Bluetooth detection capabilities, especially for Bluetooth Classic devices.

Key features:
- Multiple detection methods:
  - `hcitool inq` for Bluetooth Classic devices
  - `hcidump` for monitoring Bluetooth traffic
  - `hcitool lescan` for BLE devices
- Systemd service for automatic startup and management
- Configurable cooldown period
- Support for multiple authorized devices
- Robust error handling and automatic restart

## Configuration

### ESP32 Implementation
This project uses a separate configuration file to keep sensitive information out of version control. To configure the project:

1. **Copy the example configuration file**
   ```
   cp config.example.h config.h
   ```

2. **Edit the configuration file** (`config.h`) with your settings:

   ```cpp
   // Wi-Fi credentials
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASSWORD "your_wifi_password"
   
   // Network broadcast address
   #define BROADCAST_IP_1 192
   #define BROADCAST_IP_2 168
   #define BROADCAST_IP_3 0
   #define BROADCAST_IP_4 255
   
   // PC MAC address for Wake-on-LAN
   #define PC_MAC_ADDRESS { 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX }
   
   // Authorized BLE devices
   #define ALLOWED_BLE_MACS { \
     "xx:xx:xx:xx:xx:xx", /* Device name */ \
     "yy:yy:yy:yy:yy:yy"  /* Another device */ \
   }
   
   // Authorized Bluetooth Classic devices (for dual mode and Classic only implementations)
   #define ALLOWED_CLASSIC_MACS { \
     "xx:xx:xx:xx:xx:xx", /* Device name */ \
     "yy:yy:yy:yy:yy:yy"  /* Another device */ \
   }
   
   // Timing settings
   #define BLE_SCAN_INTERVAL 1
   #define CLASSIC_SCAN_CYCLES 5
   #define WAKE_COOLDOWN 30000
   ```

### Raspberry Pi Implementation
To configure the Raspberry Pi implementation:

1. **Run the setup script**
   ```bash
   cd turn-on-pc-via-bluetooth-raspberry-pi
   sudo ./setup_monitor_service.sh
   ```

2. **Follow the prompts** to:
   - Enter MAC addresses of authorized Bluetooth devices
   - Enter the MAC address of the target PC
   - The script will automatically configure and start the service

## Installation

### ESP32 Implementation
1. Install the Arduino IDE and ESP32 board support
2. Install required libraries through the Arduino Library Manager
3. Clone or download this repository
4. Copy `config.example.h` to `config.h` and edit with your settings
5. Open the desired .ino file in Arduino IDE
6. Connect your ESP32 to your computer
7. Select the correct board and port in Arduino IDE
8. Upload the sketch to your ESP32

### Raspberry Pi Implementation
1. Install Raspberry Pi OS Lite (or your preferred Linux distribution)
2. Install required packages:
   ```bash
   sudo apt update && sudo apt upgrade -y
   sudo apt install -y bluez bluez-hcidump bluetooth
   ```
3. Run the setup script directly from the repository:
   ```bash
   bash <(curl -fsSL https://raw.githubusercontent.com/dragaoazuljr/esp32-turn-on-pc-bluetooth/master/turn-on-pc-via-bluetooth-raspberry-pi/setup_monitor_service.sh)
   ```

## Usage

1. Ensure your PC has Wake-on-LAN enabled in BIOS/UEFI settings
2. Power the ESP32/Raspberry Pi
3. Place the device within range of your Bluetooth devices
4. When you turn on an authorized Bluetooth device, it will detect it and send a Wake-on-LAN packet to your PC

## Troubleshooting

### ESP32 Implementation
- **ESP32 not detecting devices**: Ensure the MAC addresses are correctly entered and in lowercase
- **PC not turning on**: Verify Wake-on-LAN is properly configured in your PC's BIOS/UEFI and network adapter settings
- **Connection issues**: Check Wi-Fi credentials and ensure the ESP32 is connected to the network
- **Multiple wake signals**: Adjust the cooldown period if needed

### Raspberry Pi Implementation
- **Device not detected**: Run `hcidump -X` to monitor Bluetooth traffic and identify the correct MAC address
- **DualShock 4 specific**: The controller may use different MAC addresses when turning on. Add both MACs to the authorized list
- **Service not starting**: Check status with `sudo systemctl status monitorbt.service`
- **Permission issues**: Ensure the script has execute permissions (`chmod +x setup_monitor_service.sh`)

### 🛠️ Common Upload Issues

#### ❌ Unknown device or board not compiling

**Fix:** In Arduino IDE, go to `Tools > Board` and select **ESP32 Dev Module**

#### ❌ "Sketch too big" / exceeds program storage space

**Fix:** In Arduino IDE, go to `Tools > Partition Scheme` and select **"Huge APP (3MB No OTA/1MB SPIFFS)"**

#### ❌ Permission denied on /dev/ttyUSB0

**Fix:** Run the following command to add your user to the `dialout` group (log out and back in afterward):

```bash
sudo usermod -a -G dialout $USER
```

#### ❌ Unable to verify flash chip / upload stuck at "Connecting..."

**Fix:** While uploading, press and **hold the BOOT button** on the ESP32 until uploading starts

---

### 💡 Still stuck?

Try the ancient art of turning it off and on again. Or better:

**"Chat GPThrough your way of it"** — a sacred ritual involving asking ChatGPT and hoping for magic 

## License

This project is open-source and available for personal and commercial use.