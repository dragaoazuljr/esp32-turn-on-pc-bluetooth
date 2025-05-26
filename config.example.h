// config.example.h - Arquivo de configuração de exemplo para o projeto ESP32 Turn On PC via Bluetooth
// Copie este arquivo para config.h e edite com suas configurações

#ifndef CONFIG_H
#define CONFIG_H

// ======== CONFIGURAÇÕES DE WI-FI ========
#define WIFI_SSID "seu_wifi_ssid"
#define WIFI_PASSWORD "sua_senha_wifi"

// ======== CONFIGURAÇÕES DE REDE ========
// Endereço de broadcast da rede local (substitua pelo seu)
#define BROADCAST_IP_1 192
#define BROADCAST_IP_2 168
#define BROADCAST_IP_3 0
#define BROADCAST_IP_4 255

// ======== CONFIGURAÇÕES DO PC ALVO ========
// Endereço MAC do PC para Wake-on-LAN (formato: 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX)
#define PC_MAC_ADDRESS { 0x3C, 0x52, 0x82, 0xAB, 0xCD, 0xEF }

// ======== DISPOSITIVOS BLUETOOTH AUTORIZADOS ========
// Lista de MACs BLE autorizados (em minúsculas)
#define ALLOWED_BLE_MACS { \
  "xx:xx:xx:xx:xx:xx", /* Nome do dispositivo 1 */ \
  "yy:yy:yy:yy:yy:yy"  /* Nome do dispositivo 2 */ \
}

// Lista de MACs Bluetooth Classic autorizados (em minúsculas)
#define ALLOWED_CLASSIC_MACS { \
  "xx:xx:xx:xx:xx:xx", /* Nome do dispositivo 1 */ \
  "yy:yy:yy:yy:yy:yy"  /* Nome do dispositivo 2 */ \
}

// ======== CONFIGURAÇÕES DE TEMPORIZAÇÃO ========
// Intervalo de escaneamento BLE (em segundos)
#define BLE_SCAN_INTERVAL 1

// Ciclos de escaneamento Bluetooth Classic (cada ciclo ~1.28s)
#define CLASSIC_SCAN_CYCLES 5

// Tempo de espera entre envios de Wake-on-LAN (em milissegundos)
#define WAKE_COOLDOWN 30000

#endif // CONFIG_H