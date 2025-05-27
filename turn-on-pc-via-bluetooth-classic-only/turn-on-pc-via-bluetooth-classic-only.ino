#include <WiFi.h>
#include <WiFiUdp.h>
#include <esp_wifi.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h" // Para ESP_BT_GAP_DEV_PROP_RSSI etc.
#include "esp_bt_device.h"  // Para esp_bt_dev_get_address_type

#include "../config.h"

// Wi-Fi credentials
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// Lista de MACs autorizados (Bluetooth Classic)
const char* allowedClassicMacs[] = ALLOWED_CLASSIC_MACS;
const int numClassicMacs = sizeof(allowedClassicMacs) / sizeof(allowedClassicMacs[0]);

// MAC address do PC para Wake-on-LAN
const uint8_t pcMacAddress[] = PC_MAC_ADDRESS;

// Configurações de rede e UDP
IPAddress broadcastIP(BROADCAST_IP_1, BROADCAST_IP_2, BROADCAST_IP_3, BROADCAST_IP_4);
WiFiUDP udp;

unsigned long lastWakeSent = 0;
const unsigned long wakeCooldown = WAKE_COOLDOWN; // Assegure-se que WAKE_COOLDOWN está definido em config.h

void sendWakeOnLan(const uint8_t* mac) {
  uint8_t magicPacket[102];
  memset(magicPacket, 0xFF, 6);
  for (int i = 0; i < 16; ++i) {
    memcpy(&magicPacket[6 + i * 6], mac, 6);
  }
  udp.beginPacket(broadcastIP, WOL_PORT); // Assegure-se que WOL_PORT está definido em config.h
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

// Callback para eventos Bluetooth Classic
void btGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  switch (event) {
    case ESP_BT_GAP_DISC_RES_EVT: {
      char bda_str[18];
      sprintf(bda_str, "%02x:%02x:%02x:%02x:%02x:%02x",
              param->disc_res.bda[0], param->disc_res.bda[1], param->disc_res.bda[2],
              param->disc_res.bda[3], param->disc_res.bda[4], param->disc_res.bda[5]);

      int8_t rssi = 0; // Default RSSI
      char device_name[ESP_BT_GAP_MAX_BDNAME_LEN + 1] = {0}; // Default name

      for (int i = 0; i < param->disc_res.num_prop; i++) {
        if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_RSSI) {
          rssi = *(int8_t*)(param->disc_res.prop[i].val);
        } else if (param->disc_res.prop[i].type == ESP_BT_GAP_DEV_PROP_BDNAME) {
           strncpy(device_name, (char*)param->disc_res.prop[i].val, ESP_BT_GAP_MAX_BDNAME_LEN);
           device_name[ESP_BT_GAP_MAX_BDNAME_LEN] = '\0'; // Ensure null termination
        }
      }
      Serial.printf("[Classic] Dispositivo encontrado: %s (%s) RSSI: %d\n", bda_str, (strlen(device_name) > 0 ? device_name : "N/A"), rssi);

      if (isAllowedMac(bda_str, allowedClassicMacs, numClassicMacs)) {
        if (millis() - lastWakeSent > wakeCooldown || lastWakeSent == 0) {
          Serial.println("[Classic] Dispositivo autorizado detectado! Enviando WoL...");
          sendWakeOnLan(pcMacAddress);
          lastWakeSent = millis();
        } else {
          Serial.println("[Classic] Dispositivo autorizado, mas em cooldown.");
        }
      }
      break;
    }
    case ESP_BT_GAP_DISC_STATE_CHANGED_EVT: {
      if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STOPPED) {
        Serial.println("[Classic] Scan (Inquiry) stopped. Restarting scan...");
        // Reinicia o scan aqui para manter a busca contínua
        esp_err_t ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
        if (ret != ESP_OK) {
            Serial.printf("[Classic] Erro ao reiniciar scan: %s\n", esp_err_to_name(ret));
        }
      } else if (param->disc_st_chg.state == ESP_BT_GAP_DISCOVERY_STARTED) {
        Serial.println("[Classic] Scan (Inquiry) started.");
      }
      break;
    }
    // O evento ESP_BT_GAP_DISC_CMPL_EVT não é tipicamente usado para o fim do *inquiry* de BT Clássico.
    // O fim do período de inquiry é ESP_BT_GAP_DISCOVERY_STOPPED.
    default:
      Serial.printf("[Classic] Evento BT Nao Tratado: %d (0x%X)\n", event, event);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Conecta ao Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi...");
  unsigned long wifiConnectStart = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - wifiConnectStart > 20000) { // Timeout de 20 segundos
        Serial.println("\nFalha ao conectar ao Wi-Fi!");
        // Você pode querer parar aqui ou tentar reiniciar
        return;
    }
  }
  Serial.println("\nWi-Fi conectado!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Broadcast IP for WoL: ");
  Serial.println(broadcastIP);


  // Inicializa UDP (aqui ou antes do WoL, mas begin() é importante)
  if (udp.begin(WOL_PORT)) { // Tenta usar uma porta específica para escuta, se necessário
    Serial.printf("UDP Listener iniciado na porta %d\n", WOL_PORT);
  } else {
    Serial.println("Falha ao iniciar UDP Listener.");
    // Se o ESP32 só envia WoL e não precisa escutar, este begin(porta) é menos crítico
    // mas udp.beginPacket() implicitamente chama begin() se não foi chamado.
    // Para envio, udp.begin() sem argumentos ou udp.begin(0) é suficiente.
    // No seu caso, udp.begin(WOL_PORT) no setup como você tinha é OK se for pra escutar também.
    // Se for apenas para enviar, não precisa de uma porta específica no begin().
    // Vamos manter como estava, pois pode não ser um problema e o beginPacket define o destino.
  }


  // Inicializa Bluetooth Classic
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  esp_err_t ret;

  // Soltar memória do modo BLE se não for usar, pode ajudar (OPCIONAL, MAS RECOMENDADO)
  // Faça isso ANTES de esp_bt_controller_init se você não for usar BLE.
  // Isso pode ajudar a colocar o controlador em um estado mais "limpo".
  #if CONFIG_IDF_TARGET_ESP32 // Aplicável principalmente ao ESP32 original
    esp_err_t release_ret = esp_bt_controller_mem_release(ESP_BT_MODE_BLE);
    if (release_ret == ESP_OK) {
        Serial.println("Memória do controlador BLE liberada.");
    } else {
        Serial.printf("Falha ao liberar memória BLE: %s (pode ser normal se BLE não estava ativo)\n", esp_err_to_name(release_ret));
    }
  #endif


  ret = esp_bt_controller_init(&bt_cfg);
  // MODIFICAÇÃO AQUI:
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao inicializar controlador BT: %s\n", esp_err_to_name(ret));
    return;
  }
  if (ret == ESP_ERR_INVALID_STATE) {
    Serial.println("Controlador BT já estava inicializado (provavelmente pelo Wi-Fi). Continuando...");
  } else {
    Serial.println("Controlador BT inicializado com sucesso.");
  }

  ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
  // MODIFICAÇÃO AQUI TAMBÉM (para consistência e log)
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao ativar controlador BT: %s\n", esp_err_to_name(ret));
    return;
  }
   if (ret == ESP_ERR_INVALID_STATE) {
    Serial.println("Controlador BT já estava ativo. Continuando...");
  } else {
    Serial.println("Controlador BT ativado com sucesso para modo Classic.");
  }


  ret = esp_bluedroid_init();
  // MODIFICAÇÃO AQUI TAMBÉM (para consistência e log)
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao inicializar Bluedroid: %s\n", esp_err_to_name(ret));
    return;
  }
  if (ret == ESP_ERR_INVALID_STATE) {
    Serial.println("Bluedroid já estava inicializado. Continuando...");
  } else {
    Serial.println("Bluedroid inicializado com sucesso.");
  }

  ret = esp_bluedroid_enable();
  // MODIFICAÇÃO AQUI TAMBÉM (para consistência e log)
  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    Serial.printf("Erro ao ativar Bluedroid: %s\n", esp_err_to_name(ret));
    return;
  }
  if (ret == ESP_ERR_INVALID_STATE) {
    Serial.println("Bluedroid já estava ativo. Continuando...");
  } else {
    Serial.println("Bluedroid ativado com sucesso.");
  }

  // Registra o callback
  ret = esp_bt_gap_register_callback(btGapCallback);
  if (ret != ESP_OK) {
    Serial.printf("Erro ao registrar callback GAP BT: %s\n", esp_err_to_name(ret));
    return;
  }

  // Inicia escaneamento (Inquiry)
  // Duração: inquiry_len * 1.28 segundos. 10 * 1.28s = 12.8s
  // num_responses: 0 para ilimitado (dentro da duração)
  ret = esp_bt_gap_start_discovery(ESP_BT_INQ_MODE_GENERAL_INQUIRY, 10, 0);
  if (ret != ESP_OK) {
    Serial.printf("Erro ao iniciar descoberta BT: %s\n", esp_err_to_name(ret));
    return;
  }

  Serial.println("Sistema pronto: Wi-Fi e Bluetooth Classic inicializados. Iniciando scan...");
}

void loop() {
  // O scan agora é reiniciado automaticamente pelo callback btGapCallback
  // quando ESP_BT_GAP_DISCOVERY_STOPPED é recebido.
  // Portanto, não precisamos mais chamar esp_bt_gap_start_discovery() aqui.
  delay(100); // Delay mínimo para manter o watchdog alimentado e permitir outras tarefas.
}