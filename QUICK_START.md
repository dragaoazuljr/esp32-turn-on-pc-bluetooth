# Quick Start Guide

This guide will help you quickly get started with the ESP32 Turn On PC via Bluetooth project.

## Choose Your Implementation

The project offers four different implementations:

1. **BLE Only** (ESP32)
   - Best for modern controllers with BLE support
   - Lowest power consumption
   - Fastest response time

2. **Dual Mode** (ESP32)
   - Supports both BLE and Bluetooth Classic
   - Good for mixed device environments
   - Medium power consumption

3. **Bluetooth Classic Only** (ESP32)
   - Best for older controllers
   - Only detects devices in pairing mode
   - Medium power consumption

4. **Raspberry Pi**
   - Best for Bluetooth Classic devices
   - Can detect devices even when not in pairing mode
   - Most robust and flexible
   - Highest power consumption

> **Important**: The ESP32 implementation can only detect Bluetooth Classic devices (like DualShock 4) when they are in pairing mode. The Raspberry Pi implementation can detect these devices both in pairing mode and during normal operation, making it more convenient for daily use.

## Quick Start: ESP32 Implementation

1. **Install Required Software**
   ```bash
   # Install Arduino IDE
   # Install ESP32 board support
   # Install required libraries
   ```

2. **Configure the Project**
   ```bash
   cp config.example.h config.h
   # Edit config.h with your settings
   ```

3. **Upload to ESP32**
   - Open the desired .ino file
   - Select your board
   - Click Upload

## Quick Start: Raspberry Pi Implementation

1. **Install Required Software**
   ```bash
   sudo apt-get update
   sudo apt-get install bluez wakeonlan
   ```

2. **Run Setup Script**
   ```bash
   cd turn-on-pc-via-bluetooth-raspberry-pi
   sudo ./setup_monitor_service.sh
   ```

3. **Follow the Prompts**
   - Enter MAC addresses of authorized devices
   - Enter MAC address of target PC
   - The script will configure and start the service

## Finding Device MAC Addresses

### For ESP32 Implementation
1. Use a Bluetooth scanner app on your phone
2. Look for the device in the scanner
3. Note the MAC address shown

### For Raspberry Pi Implementation
1. Run `hcidump -X` on the Raspberry Pi
2. Turn on your Bluetooth device
3. Look for the MAC address in the output
4. For DualShock 4, you may see two different MACs:
   - One when in pairing mode
   - Another when normally powered on
   - Add both to the authorized list

## Testing the Setup

1. **Enable Wake-on-LAN**
   - Enter your PC's BIOS/UEFI
   - Enable Wake-on-LAN
   - Save and exit

2. **Test the Connection**
   - Power on your Bluetooth device
   - The ESP32/Raspberry Pi should detect it
   - Your PC should turn on

## Troubleshooting

### ESP32 Issues
- **Not detecting devices**: Check MAC addresses
- **Connection issues**: Verify Wi-Fi settings
- **Upload problems**: Hold BOOT button during upload

### Raspberry Pi Issues
- **Service not starting**: Check status with `sudo systemctl status monitorbt.service`
- **Device not detected**: Run `hcidump -X` to monitor Bluetooth traffic
- **Permission issues**: Ensure script has execute permissions

## Next Steps

1. **Fine-tune Settings**
   - Adjust scan intervals
   - Modify cooldown periods
   - Add more authorized devices

2. **Enhance Security**
   - Review authorized device list
   - Update Wi-Fi credentials
   - Consider additional security measures

3. **Optimize Performance**
   - Monitor power consumption
   - Adjust detection parameters
   - Fine-tune cooldown periods

## Need Help?

- Check the full documentation in README.md
- Review technical details in TECHNICAL_DETAILS.md
- Open an issue on GitHub
- Ask in the discussions section
