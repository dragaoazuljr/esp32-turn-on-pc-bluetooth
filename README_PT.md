# ESP32 Ligar PC via Bluetooth

Este projeto usa um microcontrolador ESP32 para enviar automaticamente um pacote Wake-on-LAN (WoL) para ligar um PC quando dispositivos Bluetooth específicos são detectados nas proximidades. É perfeito para ligar automaticamente seu PC quando você liga um controle de jogo ou outro dispositivo Bluetooth.

## Visão Geral do Projeto

Este projeto permite que você ligue automaticamente seu PC usando um ESP32 sempre que dispositivos Bluetooth específicos ficam online nas proximidades — perfeito para ligar seu PC com um controle.

O repositório fornece **quatro implementações**, cada uma adaptada para diferentes protocolos Bluetooth e hardware:

1. **Implementação Apenas BLE** (`turn-on-pc-via-bluetooth.ino`)

   * Usa **Bluetooth Low Energy (BLE)** exclusivamente.
   * Adequado para **controles modernos** que transmitem anúncios BLE mesmo após o pareamento (ex: *8bitdo Ultimate 2 modo Bluetooth*).
   * Oferece **resposta mais rápida** e **menor consumo de energia**.

2. **Implementação Modo Duplo** (`turn-on-pc-via-bluetooth-ble-classic.ino`)

   * Alterna entre varredura **BLE e Bluetooth Classic**.
   * Projetado para **maior compatibilidade**, incluindo **dispositivos mais antigos** que não suportam BLE.
   * Pode detectar dispositivos como **DualShock 4** ou **8bitdo SN30 Pro**, **mas apenas quando estão em modo de pareamento** (ou seja, descobrível).

3. **Implementação Apenas Bluetooth Classic** (`turn-on-pc-via-bluetooth-classic-only.ino`)

   * Usa **Bluetooth Classic** exclusivamente.
   * Otimizado para **dispositivos mais antigos** que só suportam Bluetooth Classic.
   * Varre continuamente dispositivos em modo de pareamento/descoberta.
   * Ideal para usuários que só precisam detectar dispositivos Bluetooth Classic.

4. **Implementação Raspberry Pi** (`turn-on-pc-via-bluetooth-raspberry-pi/`)

   * Usa **Raspberry Pi** com Linux para detecção Bluetooth aprimorada.
   * Implementa **múltiplos métodos de detecção**:
     - `hcitool inq` para dispositivos Bluetooth Classic
     - `hcidump` para monitoramento de tráfego Bluetooth
     - `hcitool lescan` para dispositivos BLE
   * **Melhor solução para dispositivos Bluetooth Classic** que não funcionam com ESP32 fora do modo de descoberta.
   * Pode detectar dispositivos como **DualShock 4** mesmo quando não estão em modo de pareamento.
   * Mais robusto e flexível, mas requer mais energia e configuração.

---

## Entendendo BLE vs Bluetooth Classic

Alguns dispositivos Bluetooth suportam **BLE** e continuarão transmitindo sua presença mesmo após serem pareados com outro dispositivo. Estes dispositivos podem ser detectados **imediatamente ao ligar**, tornando o script apenas BLE ideal.

No entanto, **muitos controles mais antigos** só suportam **Bluetooth Classic**, e só aparecem durante uma **janela curta enquanto estão em modo de pareamento**. Isso significa que o ESP32 não pode detectá-los a menos que o dispositivo seja explicitamente colocado em modo de pareamento.

### ✅ Dispositivos que funcionam bem com BLE:

* **8bitdo Ultimate 2 (modo BLE)**
* A maioria dos rastreadores fitness modernos, teclados e periféricos de baixa energia
  ➡ Detectado mesmo após pareamento — ideal para o script **Apenas BLE**.

### ⚠️ Dispositivos que requerem Bluetooth Classic:

* **DualShock 4**
* **8bitdo SN30 Pro**, modelos mais antigos da 8bitdo
  ➡ Visível apenas durante o **modo de pareamento** — use o script **Modo Duplo** ou **Apenas Classic**.

> **Nota Importante**: Ao usar a implementação ESP32, dispositivos Bluetooth Classic só podem ser detectados quando estão em modo de pareamento (pairable). No entanto, a implementação Raspberry Pi pode detectar estes dispositivos tanto quando estão em modo de pareamento QUANTO quando estão ligados normalmente. Isso torna a solução Raspberry Pi mais conveniente para uso diário com controles como o DualShock 4.

---

### 🔍 Como saber se seu dispositivo suporta BLE?

Você pode testar usando um celular Android:

1. Abra o **menu Bluetooth**.
2. Ligue seu dispositivo (controle, etc.).
3. Se o dispositivo **aparecer na lista enquanto já está pareado ou após ligar**, provavelmente suporta BLE.
4. Se **só aparecer enquanto está em modo de pareamento**, provavelmente é apenas Bluetooth Classic.

Dispositivos BLE frequentemente anunciam sua presença como conectável, mesmo se já estiverem conectados a outro dispositivo.

## Requisitos de Hardware

### Implementação ESP32
- Placa de desenvolvimento ESP32 (ESP32-WROOM, ESP32-WROVER, ou similar)
- Fonte de alimentação para o ESP32 (USB ou externa)
- PC com Wake-on-LAN habilitado na BIOS/UEFI

### Implementação Raspberry Pi
- Raspberry Pi (qualquer modelo com Bluetooth)
- Fonte de alimentação para o Raspberry Pi
- PC com Wake-on-LAN habilitado na BIOS/UEFI

## Requisitos de Software

### Implementação ESP32
- Arduino IDE
- Pacote de suporte para ESP32 no Arduino
- Bibliotecas necessárias:
  - WiFi.h
  - WiFiUdp.h
  - BLEDevice.h (para implementações BLE e modo duplo)
  - BLEUtils.h (para implementações BLE e modo duplo)
  - BLEScan.h (para implementações BLE e modo duplo)
  - esp_bt.h (para implementações modo duplo e apenas Classic)
  - esp_bt_main.h (para implementações modo duplo e apenas Classic)
  - esp_gap_bt_api.h (para implementações modo duplo e apenas Classic)

### Implementação Raspberry Pi
- Raspberry Pi OS (ou qualquer distribuição Linux)
- Pacotes necessários:
  - bluez (para ferramentas Bluetooth)
  - wakeonlan (para pacotes WoL)

## Detalhes da Implementação

### Implementação Apenas BLE (`turn-on-pc-via-bluetooth.ino`)

Esta implementação varre apenas dispositivos BLE. É mais simples e usa menos energia, mas pode não detectar dispositivos Bluetooth Classic mais antigos.

Características principais:
- Varre dispositivos BLE em intervalos regulares
- Verifica dispositivos detectados contra uma lista autorizada
- Envia pacote Wake-on-LAN quando um dispositivo autorizado é encontrado
- Implementa um período de cooldown para evitar múltiplos sinais de wake

### Implementação Modo Duplo (`turn-on-pc-via-bluetooth-ble-classic.ino`)

Esta implementação varre tanto dispositivos BLE quanto Bluetooth Classic, fornecendo maior compatibilidade com vários controles e dispositivos.

Características principais:
- Alterna entre varredura BLE e Bluetooth Classic
- Mantém listas separadas de dispositivos autorizados para cada protocolo
- Usa callbacks para detecção de dispositivos Bluetooth Classic
- Implementa um período de cooldown para evitar múltiplos sinais de wake
- Fornece logging mais detalhado

### Implementação Apenas Bluetooth Classic (`turn-on-pc-via-bluetooth-classic-only.ino`)

Esta implementação foca exclusivamente em dispositivos Bluetooth Classic, otimizada para controles e periféricos mais antigos.

Características principais:
- Usa apenas varredura Bluetooth Classic (sem BLE)
- Reinicia continuamente a varredura quando completada
- Tenta recuperar nomes de dispositivos quando disponíveis
- Implementa reconexão automática Wi-Fi
- Fornece logging detalhado de dispositivos detectados

### Implementação Raspberry Pi (`turn-on-pc-via-bluetooth-raspberry-pi/`)

Esta implementação usa um Raspberry Pi para fornecer capacidades aprimoradas de detecção Bluetooth, especialmente para dispositivos Bluetooth Classic.

Características principais:
- Múltiplos métodos de detecção:
  - `hcitool inq` para dispositivos Bluetooth Classic
  - `hcidump` para monitoramento de tráfego Bluetooth
  - `hcitool lescan` para dispositivos BLE
- Serviço systemd para inicialização e gerenciamento automático
- Período de cooldown configurável
- Suporte para múltiplos dispositivos autorizados
- Tratamento robusto de erros e reinicialização automática

## Configuração

### Implementação ESP32
Este projeto usa um arquivo de configuração separado para manter informações sensíveis fora do controle de versão. Para configurar o projeto:

1. **Copie o arquivo de configuração de exemplo**
   ```
   cp config.example.h config.h
   ```

2. **Edite o arquivo de configuração** (`config.h`) com suas configurações:

   ```cpp
   // Credenciais Wi-Fi
   #define WIFI_SSID "seu_wifi_ssid"
   #define WIFI_PASSWORD "sua_senha_wifi"
   
   // Endereço de broadcast da rede
   #define BROADCAST_IP_1 192
   #define BROADCAST_IP_2 168
   #define BROADCAST_IP_3 0
   #define BROADCAST_IP_4 255
   
   // Endereço MAC do PC para Wake-on-LAN
   #define PC_MAC_ADDRESS { 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX }
   
   // Dispositivos BLE autorizados
   #define ALLOWED_BLE_MACS { \
     "xx:xx:xx:xx:xx:xx", /* Nome do dispositivo */ \
     "yy:yy:yy:yy:yy:yy"  /* Outro dispositivo */ \
   }
   
   // Dispositivos Bluetooth Classic autorizados (para implementações modo duplo e apenas Classic)
   #define ALLOWED_CLASSIC_MACS { \
     "xx:xx:xx:xx:xx:xx", /* Nome do dispositivo */ \
     "yy:yy:yy:yy:yy:yy"  /* Outro dispositivo */ \
   }
   
   // Configurações de tempo
   #define BLE_SCAN_INTERVAL 1
   #define CLASSIC_SCAN_CYCLES 5
   #define WAKE_COOLDOWN 30000
   ```

### Implementação Raspberry Pi
Para configurar a implementação Raspberry Pi:

1. **Execute o script de configuração**
   ```bash
   cd turn-on-pc-via-bluetooth-raspberry-pi
   sudo ./setup_monitor_service.sh
   ```

2. **Siga os prompts** para:
   - Inserir endereços MAC dos dispositivos Bluetooth autorizados
   - Inserir o endereço MAC do PC alvo
   - O script configurará e iniciará o serviço automaticamente

## Instalação

### Implementação ESP32
1. Instale o Arduino IDE e o suporte para ESP32
2. Instale as bibliotecas necessárias através do Gerenciador de Bibliotecas do Arduino
3. Clone ou baixe este repositório
4. Copie `config.example.h` para `config.h` e edite com suas configurações
5. Abra o arquivo .ino desejado no Arduino IDE
6. Conecte seu ESP32 ao computador
7. Selecione a placa e porta corretas no Arduino IDE
8. Faça upload do sketch para o ESP32

### Implementação Raspberry Pi
1. Instale o Raspberry Pi OS Lite (ou sua distribuição Linux preferida)
2. Instale os pacotes necessários:
   ```bash
   sudo apt update && sudo apt upgrade -y
   sudo apt install -y bluez bluez-hcidump bluetooth
   ```
3. Execute o script de configuração diretamente do repositório:
   ```bash
   bash <(curl -fsSL https://raw.githubusercontent.com/dragaoazuljr/esp32-turn-on-pc-bluetooth/master/turn-on-pc-via-bluetooth-raspberry-pi/setup_monitor_service.sh)
   ```

## Uso

1. Certifique-se de que seu PC tem Wake-on-LAN habilitado nas configurações da BIOS/UEFI
2. Alimente o ESP32/Raspberry Pi
3. Coloque o dispositivo dentro do alcance de seus dispositivos Bluetooth
4. Quando você ligar um dispositivo Bluetooth autorizado, ele será detectado e enviará um pacote Wake-on-LAN para seu PC

## Solução de Problemas

### Implementação ESP32
- **ESP32 não detecta dispositivos**: Verifique se os endereços MAC estão corretamente inseridos e em minúsculas
- **PC não liga**: Verifique se o Wake-on-LAN está corretamente configurado na BIOS/UEFI e nas configurações do adaptador de rede
- **Problemas de conexão**: Verifique as credenciais Wi-Fi e certifique-se de que o ESP32 está conectado à rede
- **Múltiplos sinais de wake**: Ajuste o período de cooldown se necessário

### Implementação Raspberry Pi
- **Dispositivo não detectado**: Execute `hcidump -X` para monitorar o tráfego Bluetooth e identificar o endereço MAC correto
- **Específico para DualShock 4**: O controle pode usar diferentes endereços MAC ao ligar. Adicione ambos os MACs à lista autorizada
- **Serviço não inicia**: Verifique o status com `sudo systemctl status monitorbt.service`
- **Problemas de permissão**: Certifique-se de que o script tem permissões de execução (`chmod +x setup_monitor_service.sh`)

### 🛠️ Problemas Comuns de Upload

#### ❌ Dispositivo desconhecido ou placa não compila

**Solução:** No Arduino IDE, vá para `Tools > Board` e selecione **ESP32 Dev Module**

#### ❌ "Sketch muito grande" / excede o espaço de armazenamento do programa

**Solução:** No Arduino IDE, vá para `Tools > Partition Scheme` e selecione **"Huge APP (3MB No OTA/1MB SPIFFS)"**

#### ❌ Permissão negada em /dev/ttyUSB0

**Solução:** Execute o seguinte comando para adicionar seu usuário ao grupo `dialout` (faça logout e login novamente depois):

```bash
sudo usermod -a -G dialout $USER
```

#### ❌ Não é possível verificar o chip flash / upload travado em "Conectando..."

**Solução:** Durante o upload, pressione e **segure o botão BOOT** no ESP32 até o upload começar

---

### 💡 Ainda com problemas?

Tente a antiga arte de desligar e ligar novamente. Ou melhor:

**"Chat GPThrough your way of it"** — um ritual sagrado envolvendo perguntar ao ChatGPT e esperar por mágica 

## Licença

Este projeto é open-source e disponível para uso pessoal e comercial. 