# Technical Documentation: ESP32 Turn On PC via Bluetooth

This document provides detailed technical information about the implementation of the ESP32 Turn On PC via Bluetooth project.

## Wake-on-LAN Protocol

Wake-on-LAN (WoL) is a networking standard that allows a computer to be turned on by a network message. The message is usually sent as a broadcast UDP datagram containing the target computer's MAC address in a specific format known as a "magic packet".

### Magic Packet Structure

The magic packet consists of:
- 6 bytes of `0xFF` (synchronization stream)
- 16 repetitions of the target MAC address (96 bytes)

Total packet size: 102 bytes

## Implementation Details

### BLE Only Implementation

#### Key Components

1. **Wi-Fi Connection**
   - Connects to the specified Wi-Fi network to enable UDP packet transmission
   - Uses `WiFi.h` library for connection management

2. **BLE Scanning**
   - Uses `BLEDevice`, `BLEUtils`, and `BLEScan` libraries
   - Performs active scanning for better device detection
   - Scans for the specified duration (`scanInterval`)

3. **MAC Address Verification**
   - Compares detected device MAC addresses against the authorized list
   - Uses case-insensitive comparison

4. **Wake-on-LAN Packet Generation**
   - Creates the magic packet with 6 bytes of `0xFF` followed by 16 repetitions of the target MAC
   - Sends the packet via UDP to the network broadcast address on port 7

5. **Cooldown Mechanism**
   - Implements a 30-second delay after sending a WoL packet to prevent multiple wake signals

#### Main Loop Flow

1. Start BLE scan
2. Process scan results
3. Check each device against authorized list
4. If authorized device found, send WoL packet
5. Wait for cooldown period
6. Clear scan results and repeat

### Dual Mode Implementation

#### Additional Components

1. **Bluetooth Classic Support**
   - Uses ESP32's Bluetooth Classic API (`esp_bt.h`, `esp_bt_main.h`, `esp_gap_bt_api.h`)
   - Implements callback function for device discovery events
   - Maintains separate list of authorized Bluetooth Classic devices
   - **Limitation**: Can only detect devices when they are in pairing mode (pairable)

2. **Dual Mode Controller Configuration**
   - Initializes Bluetooth controller in dual mode (BLE + Classic)
   - Configures proper event handling for both protocols

3. **Alternating Scan Strategy**
   - Alternates between BLE and Bluetooth Classic scanning to prevent resource conflicts
   - Uses state tracking to manage scan mode transitions

#### Bluetooth Classic Callback

The `btGapCallback` function handles Bluetooth Classic discovery events:
- `ESP_BT_GAP_DISC_RES_EVT`: Triggered when a device is found
- `ESP_BT_GAP_DISC_STATE_CHANGED_EVT`: Triggered when scan state changes

#### Scan Coordination

The implementation carefully manages the alternation between BLE and Bluetooth Classic scanning:
1. Performs BLE scan for a short duration
2. Processes BLE results
3. Switches to Bluetooth Classic scan
4. Waits for Bluetooth Classic scan to complete via callbacks
5. Returns to BLE scan after a delay

### Bluetooth Classic Only Implementation

#### Key Components

1. **Wi-Fi Connection**
   - Connects to the specified Wi-Fi network to enable UDP packet transmission
   - Uses `WiFi.h` library for connection management
   - Implements automatic reconnection if Wi-Fi connection is lost

2. **Bluetooth Classic Initialization**
   - Uses ESP32's Bluetooth Classic API (`esp_bt.h`, `esp_bt_main.h`, `esp_gap_bt_api.h`)
   - Initializes controller in Bluetooth Classic mode only (no BLE)
   - Sets device name to "ESP32-WoL" for easier identification

3. **Continuous Scanning**
   - Implements automatic restart of scanning when a scan cycle completes
   - Uses the `ESP_BT_GAP_DISC_STATE_CHANGED_EVT` event to detect when scanning stops

4. **Device Name Retrieval**
   - Attempts to retrieve and display device names when available
   - Provides more user-friendly identification of detected devices

5. **Enhanced Logging**
   - Provides detailed information about detected devices
   - Logs connection status and authorized device list on startup

#### Bluetooth Classic Callback

The `btGapCallback` function handles Bluetooth Classic discovery events:
- `ESP_BT_GAP_DISC_RES_EVT`: Processes found devices and checks against authorized list
- `ESP_BT_GAP_DISC_STATE_CHANGED_EVT`: Automatically restarts scanning when completed

#### Main Loop Flow

1. Monitor Wi-Fi connection status
2. Reconnect to Wi-Fi if disconnected
3. Most processing happens in the callback function
4. Maintain system stability with minimal delay

### Raspberry Pi Implementation

#### Key Components

1. **Multiple Detection Methods**
   - **hcitool inq**: Bluetooth Classic device discovery
     - Uses `hcitool inq` command for device scanning
     - Scans for 10 seconds with flush option
     - Detects devices in inquiry mode
   
   - **hcidump**: Bluetooth traffic monitoring
     - Uses `hcidump -X` for raw packet capture
     - Monitors all Bluetooth traffic
     - **Key Advantage**: Can detect devices even when not in discovery mode
     - This is the main difference from the ESP32 implementation
     - Makes it possible to detect controllers like DualShock 4 during normal operation
   
   - **hcitool lescan**: BLE device discovery
     - Uses `hcitool lescan` for BLE device scanning
     - Detects BLE devices and their advertisements
     - Useful for modern controllers with BLE support

2. **Process Management**
   - Runs each detection method in separate processes
   - Uses file-based IPC (`/tmp/controle_detectado.tmp`)
   - Implements proper process cleanup on exit
   - Handles process termination gracefully

3. **Systemd Service Integration**
   - Runs as a systemd service for automatic startup
   - Implements proper service dependencies
   - Handles service restart and recovery
   - Provides system logging integration

4. **Cooldown Mechanism**
   - Implements configurable cooldown period (default: 120 seconds)
   - Prevents multiple wake signals
   - Uses file-based state tracking
   - Handles edge cases and race conditions

#### Detection Process Flow

1. **hcitool inq Process**
   ```
   while true:
     execute hcitool inq
     check results against authorized MACs
     if match found:
       write MAC to detection file
       wait for cooldown
     sleep 2 seconds
   ```

2. **hcidump Process**
   ```
   while true:
     monitor hcidump output
     for each line:
       check against authorized MACs
       if match found:
         write MAC to detection file
         wait for cooldown
   ```

3. **hcitool lescan Process**
   ```
   while true:
     execute hcitool lescan
     check results against authorized MACs
     if match found:
       write MAC to detection file
       wait for cooldown
     sleep 2 seconds
   ```

4. **Main Process**
   ```
   while true:
     check detection file
     if file exists:
       read MAC
       send WoL packet
       remove detection file
       wait for cooldown
     sleep 1 second
   ```

#### Special Considerations

1. **DualShock 4 Handling**
   - The DualShock 4 controller may use different MAC addresses:
     - One MAC when in pairing mode
     - Another MAC when normally powered on
   - Solution: Add both MACs to the authorized list
   - Use `hcidump -X` to identify both MACs
   - **Key Advantage**: The Raspberry Pi implementation can detect the controller in both states:
     - When it's in pairing mode (using hcitool inq)
     - When it's normally powered on (using hcidump)

2. **Bluetooth Classic Device Detection**
   - Some devices only broadcast in discovery mode
   - Others maintain constant presence
   - Multiple detection methods increase chances of detection
   - `hcidump` can catch devices even when not in discovery mode
   - This is a significant advantage over the ESP32 implementation, which can only detect devices in pairing mode

3. **Performance Optimization**
   - Each detection method runs in parallel
   - File-based IPC minimizes overhead
   - Cooldown period prevents excessive scanning
   - Process management ensures resource cleanup

4. **Error Handling**
   - Automatic service restart on failure
   - Process monitoring and recovery
   - Logging of all significant events
   - Graceful handling of device disconnections

## Memory Management

All implementations include memory management considerations:
- BLE scan results are cleared after processing to free memory (in BLE and dual mode implementations)
- Temporary variables are used efficiently
- String operations are minimized
- Buffer sizes are optimized for the ESP32's memory constraints

## Power Considerations

The power consumption varies between implementations:

### BLE Only Implementation
- Lowest power consumption
- Efficient BLE scanning with minimal radio usage
- Recommended for battery-powered applications

### Dual Mode Implementation
- Highest power consumption
- Runs both BLE and Bluetooth Classic radios
- More frequent scanning operations
- Additional processing for dual protocol handling

### Bluetooth Classic Only Implementation
- Medium power consumption
- Higher than BLE-only but lower than dual mode
- Continuous scanning increases power usage
- More efficient than dual mode for Classic-only devices

### Raspberry Pi Implementation
- Highest power consumption among all implementations
- Runs multiple detection processes
- Continuous monitoring of Bluetooth traffic
- Requires stable power supply
- Not suitable for battery operation

## Security Considerations

This implementation has minimal security features:
- Device authorization is based solely on MAC address
- MAC addresses can potentially be spoofed
- No encryption for the Wake-on-LAN packet

For enhanced security, consider:
- Adding a secure handshake mechanism
- Implementing device pairing
- Using encrypted communication

### Configuration Security

The project now uses a separate configuration file (`config.h`) that is excluded from version control via `.gitignore`. This helps prevent accidentally committing sensitive information like:
- Wi-Fi credentials
- Network information
- Device MAC addresses

When sharing or publishing this code, only the `config.example.h` file is included, which contains placeholder values.

## Performance Optimization

Several optimizations are implemented:
- Active scanning for faster BLE device detection
- Cooldown period to prevent excessive network traffic
- Alternating scan strategy to prevent resource conflicts
- Early termination of scanning when authorized device is found

## Configuration System

The project uses a preprocessor-based configuration system that allows for easy customization without modifying the main code files:

### Configuration File Structure

The configuration is managed through a separate header file (`config.h`) that defines preprocessor macros:

```cpp
// Wi-Fi settings
#define WIFI_SSID "network_name"
#define WIFI_PASSWORD "password"

// Network settings
#define BROADCAST_IP_1 192
#define BROADCAST_IP_2 168
#define BROADCAST_IP_3 0
#define BROADCAST_IP_4 255

// Device lists
#define ALLOWED_BLE_MACS { "mac1", "mac2", "mac3" }
#define ALLOWED_CLASSIC_MACS { "mac1", "mac2" }

// Timing settings
#define BLE_SCAN_INTERVAL 1
#define CLASSIC_SCAN_CYCLES 5
#define WAKE_COOLDOWN 30000
```

### Benefits of This Approach

1. **Security**: Sensitive information is kept out of the main code files
2. **Version Control**: The actual configuration file is excluded via `.gitignore`
3. **Documentation**: The example configuration file serves as self-documenting code
4. **Compatibility**: Works with the Arduino IDE without requiring additional libraries
5. **Flexibility**: Easy to modify settings without changing the core functionality

### Alternative Approaches Considered

Other configuration approaches that could be implemented in future versions:

1. **SPIFFS/LittleFS Storage**: Store configuration in the ESP32's file system
2. **EEPROM Storage**: Store configuration in non-volatile memory
3. **Web Configuration Interface**: Allow configuration via a web browser

## Extending the Implementation

The code can be extended with:
1. **Web Configuration Interface**
   - Add ESP32 web server for configuration
   - Store settings in non-volatile memory

2. **Multiple Target Support**
   - Extend to support multiple PCs with different MAC addresses
   - Associate specific Bluetooth devices with specific PCs

3. **Advanced Power Management**
   - Implement deep sleep between scans
   - Use RTC wake-up for periodic scanning

4. **Integration with Home Automation**
   - Add MQTT support
   - Implement REST API for remote control