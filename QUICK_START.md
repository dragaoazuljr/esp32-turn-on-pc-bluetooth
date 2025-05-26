# Quick Start Guide: ESP32 Turn On PC via Bluetooth

This guide will help you quickly set up your ESP32 to turn on your PC when a specific Bluetooth device is detected.

## Prerequisites

- ESP32 development board
- USB cable for programming
- Arduino IDE installed
- ESP32 board support package installed in Arduino IDE
- PC with Wake-on-LAN enabled

## Step 1: Enable Wake-on-LAN on Your PC

1. Enter your PC's BIOS/UEFI (usually by pressing F2, F12, or Del during boot)
2. Find Power Management or similar section
3. Enable Wake-on-LAN or similar option (may be called "Power On by PCI-E" or similar)
4. Save and exit BIOS/UEFI

Next, configure your network adapter:

**Windows:**
1. Open Device Manager
2. Expand "Network adapters"
3. Right-click your network adapter and select "Properties"
4. Go to the "Power Management" tab
5. Check "Allow this device to wake the computer"
6. Go to the "Advanced" tab
7. Find "Wake on Magic Packet" or similar setting and enable it
8. Click OK

**Linux:**
```bash
sudo ethtool -s eth0 wol g
```
(Replace eth0 with your network interface name)

## Step 2: Get Your PC's MAC Address

**Windows:**
1. Open Command Prompt
2. Type `ipconfig /all`
3. Look for your network adapter and note the "Physical Address" (MAC address)

**Linux:**
```bash
ip link show
```
or
```bash
ifconfig
```

## Step 3: Get Your Bluetooth Device's MAC Address

You can use a Bluetooth scanner app on your smartphone or:

**Windows:**
1. Go to Settings > Devices > Bluetooth & other devices
2. Click on the device
3. Look for the MAC address in the device properties

**Android:**
1. Go to Settings > About phone > Status
2. Look for "Bluetooth address"

## Step 4: Configure the Project

1. Copy the example configuration file:
   ```
   cp config.example.h config.h
   ```

2. Edit the `config.h` file with your settings:

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
// Example: MAC address 3c:52:82:ab:cd:ef becomes { 0x3C, 0x52, 0x82, 0xAB, 0xCD, 0xEF }

// Authorized BLE devices
#define ALLOWED_BLE_MACS { \
  "xx:xx:xx:xx:xx:xx", /* Replace with your device's MAC */ \
}

// For dual mode implementation, also configure:
#define ALLOWED_CLASSIC_MACS { \
  "xx:xx:xx:xx:xx:xx", /* Replace with your device's MAC */ \
}
```

3. Choose which implementation you want to use:
   - `turn-on-pc-via-bluetooth.ino` (BLE only)
   - `turn-on-pc-via-bluetooth-ble-classic.ino` (BLE + Bluetooth Classic)

## Step 5: Upload to ESP32

1. Connect your ESP32 to your computer via USB
2. In Arduino IDE:
   - Select the correct board (Tools > Board > ESP32)
   - Select the correct port (Tools > Port)
   - Click the Upload button

## Step 6: Test the Setup

1. Disconnect the ESP32 from your computer
2. Connect it to a power source (USB power adapter)
3. Shut down your PC
4. Turn on your Bluetooth device
5. The ESP32 should detect the device and send a Wake-on-LAN packet
6. Your PC should power on

## Troubleshooting

### ESP32 Not Detecting Device

* Verify the MAC address is correct and in lowercase
* Check that the device is in discovery mode
* Ensure the ESP32 is within range of the Bluetooth device

### PC Not Turning On

* Verify Wake-on-LAN is properly enabled in BIOS/UEFI
* Check network adapter settings
* Ensure the PC's MAC address is correctly entered in the code
* Verify the network broadcast address is correct for your network

### ESP32 Not Connecting to Wi-Fi

* Check Wi-Fi credentials
* Ensure the ESP32 is within range of your Wi-Fi router
* Try a static IP configuration if DHCP is causing issues

### Serial Monitor Output

Connect the ESP32 to your computer and open the Serial Monitor in Arduino IDE (115200 baud) to see debugging information.

---

## 🛠️ Common Upload Issues

### ❌ Unknown device or board not compiling

**Fix:** In Arduino IDE, go to `Tools > Board` and select **ESP32 Dev Module**

### ❌ "Sketch too big" / exceeds program storage space

**Fix:** In Arduino IDE, go to `Tools > Partition Scheme` and select **"Huge APP (3MB No OTA/1MB SPIFFS)"**

### ❌ Permission denied on /dev/ttyUSB0

**Fix:** Run the following command to add your user to the `dialout` group (log out and back in afterward):

```bash
sudo usermod -a -G dialout $USER
```

### ❌ Unable to verify flash chip / upload stuck at "Connecting..."

**Fix:** While uploading, press and **hold the BOOT button** on the ESP32 until uploading starts

---

## 💡 Still stuck?

Try the ancient art of turning it off and on again. Or better:

**"Chat GPThrough your way of it"** — a sacred ritual involving asking ChatGPT and hoping for magic ✨
