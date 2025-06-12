# Documentação Técnica: ESP32 Ligar PC via Bluetooth

Este documento fornece informações técnicas detalhadas sobre a implementação do projeto ESP32 Ligar PC via Bluetooth.

## Protocolo Wake-on-LAN

Wake-on-LAN (WoL) é um padrão de rede que permite que um computador seja ligado por uma mensagem de rede. A mensagem é geralmente enviada como um datagrama UDP de broadcast contendo o endereço MAC do computador alvo em um formato específico conhecido como "pacote mágico".

### Estrutura do Pacote Mágico

O pacote mágico consiste em:
- 6 bytes de `0xFF` (stream de sincronização)
- 16 repetições do endereço MAC alvo (96 bytes)

Tamanho total do pacote: 102 bytes

## Detalhes da Implementação

### Implementação Apenas BLE

#### Componentes Principais

1. **Conexão Wi-Fi**
   - Conecta à rede Wi-Fi especificada para habilitar a transmissão de pacotes UDP
   - Usa a biblioteca `WiFi.h` para gerenciamento de conexão

2. **Varredura BLE**
   - Usa as bibliotecas `BLEDevice`, `BLEUtils` e `BLEScan`
   - Realiza varredura ativa para melhor detecção de dispositivos
   - Varre pelo tempo especificado (`scanInterval`)

3. **Verificação de Endereço MAC**
   - Compara endereços MAC de dispositivos detectados contra a lista autorizada
   - Usa comparação case-insensitive

4. **Geração de Pacote Wake-on-LAN**
   - Cria o pacote mágico com 6 bytes de `0xFF` seguidos por 16 repetições do MAC alvo
   - Envia o pacote via UDP para o endereço de broadcast da rede na porta 7

5. **Mecanismo de Cooldown**
   - Implementa um atraso de 30 segundos após enviar um pacote WoL para evitar múltiplos sinais de wake

#### Fluxo do Loop Principal

1. Inicia varredura BLE
2. Processa resultados da varredura
3. Verifica cada dispositivo contra a lista autorizada
4. Se dispositivo autorizado encontrado, envia pacote WoL
5. Aguarda período de cooldown
6. Limpa resultados da varredura e repete

### Implementação Modo Duplo

#### Componentes Adicionais

1. **Suporte Bluetooth Classic**
   - Usa a API Bluetooth Classic do ESP32 (`esp_bt.h`, `esp_bt_main.h`, `esp_gap_bt_api.h`)
   - Implementa função de callback para eventos de descoberta de dispositivos
   - Mantém lista separada de dispositivos Bluetooth Classic autorizados
   - **Limitação**: Só pode detectar dispositivos quando estão em modo de pareamento (pairable)

2. **Configuração do Controlador Modo Duplo**
   - Inicializa o controlador Bluetooth em modo duplo (BLE + Classic)
   - Configura tratamento adequado de eventos para ambos os protocolos

3. **Estratégia de Varredura Alternada**
   - Alterna entre varredura BLE e Bluetooth Classic para evitar conflitos de recursos
   - Usa rastreamento de estado para gerenciar transições de modo de varredura

#### Callback Bluetooth Classic

A função `btGapCallback` trata eventos de descoberta Bluetooth Classic:
- `ESP_BT_GAP_DISC_RES_EVT`: Acionado quando um dispositivo é encontrado
- `ESP_BT_GAP_DISC_STATE_CHANGED_EVT`: Acionado quando o estado da varredura muda

#### Coordenação de Varredura

A implementação gerencia cuidadosamente a alternância entre varredura BLE e Bluetooth Classic:
1. Realiza varredura BLE por um curto período
2. Processa resultados BLE
3. Muda para varredura Bluetooth Classic
4. Aguarda a varredura Bluetooth Classic completar via callbacks
5. Retorna à varredura BLE após um atraso

### Implementação Apenas Bluetooth Classic

#### Componentes Principais

1. **Conexão Wi-Fi**
   - Conecta à rede Wi-Fi especificada para habilitar a transmissão de pacotes UDP
   - Usa a biblioteca `WiFi.h` para gerenciamento de conexão
   - Implementa reconexão automática se a conexão Wi-Fi for perdida

2. **Inicialização Bluetooth Classic**
   - Usa a API Bluetooth Classic do ESP32 (`esp_bt.h`, `esp_bt_main.h`, `esp_gap_bt_api.h`)
   - Inicializa o controlador em modo apenas Bluetooth Classic (sem BLE)
   - Define o nome do dispositivo como "ESP32-WoL" para identificação mais fácil

3. **Varredura Contínua**
   - Implementa reinicialização automática da varredura quando um ciclo de varredura é completado
   - Usa o evento `ESP_BT_GAP_DISC_STATE_CHANGED_EVT` para detectar quando a varredura para

4. **Recuperação de Nome do Dispositivo**
   - Tenta recuperar e exibir nomes de dispositivos quando disponíveis
   - Fornece identificação mais amigável de dispositivos detectados

5. **Logging Aprimorado**
   - Fornece informações detalhadas sobre dispositivos detectados
   - Registra status de conexão e lista de dispositivos autorizados na inicialização

#### Callback Bluetooth Classic

A função `btGapCallback` trata eventos de descoberta Bluetooth Classic:
- `ESP_BT_GAP_DISC_RES_EVT`: Processa dispositivos encontrados e verifica contra a lista autorizada
- `ESP_BT_GAP_DISC_STATE_CHANGED_EVT`: Reinicia automaticamente a varredura quando completada

#### Fluxo do Loop Principal

1. Monitora status da conexão Wi-Fi
2. Reconecta ao Wi-Fi se desconectado
3. A maior parte do processamento acontece na função de callback
4. Mantém estabilidade do sistema com atraso mínimo

### Implementação Raspberry Pi

#### Componentes Principais

1. **Múltiplos Métodos de Detecção**
   - **hcitool inq**: Descoberta de dispositivos Bluetooth Classic
     - Usa o comando `hcitool inq` para varredura de dispositivos
     - Varre por 10 segundos com opção de flush
     - Detecta dispositivos em modo de inquiry
   
   - **hcidump**: Monitoramento de tráfego Bluetooth
     - Usa `hcidump -X` para captura de pacotes brutos
     - Monitora todo o tráfego Bluetooth
     - **Vantagem Principal**: Pode detectar dispositivos mesmo quando não estão em modo de descoberta
     - Esta é a principal diferença da implementação ESP32
     - Torna possível detectar controles como DualShock 4 durante operação normal
   
   - **hcitool lescan**: Descoberta de dispositivos BLE
     - Usa `hcitool lescan` para varredura de dispositivos BLE
     - Detecta dispositivos BLE e seus anúncios
     - Útil para controles modernos com suporte BLE

2. **Gerenciamento de Processos**
   - Executa cada método de detecção em processos separados
   - Usa IPC baseado em arquivo (`/tmp/controle_detectado.tmp`)
   - Implementa limpeza adequada de processos na saída
   - Trata terminação de processos graciosamente

3. **Integração com Serviço Systemd**
   - Executa como um serviço systemd para inicialização automática
   - Implementa dependências adequadas de serviço
   - Trata reinicialização e recuperação de serviço
   - Fornece integração com logging do sistema

4. **Mecanismo de Cooldown**
   - Implementa período de cooldown configurável (padrão: 120 segundos)
   - Previne múltiplos sinais de wake
   - Usa rastreamento de estado baseado em arquivo
   - Trata casos de borda e condições de corrida

#### Fluxo do Processo de Detecção

1. **Processo hcitool inq**
   ```
   enquanto verdadeiro:
     executa hcitool inq
     verifica resultados contra MACs autorizados
     se encontrou correspondência:
       escreve MAC no arquivo de detecção
       aguarda cooldown
     dorme 2 segundos
   ```

2. **Processo hcidump**
   ```
   enquanto verdadeiro:
     monitora saída do hcidump
     para cada linha:
       verifica contra MACs autorizados
       se encontrou correspondência:
         escreve MAC no arquivo de detecção
         aguarda cooldown
   ```

3. **Processo hcitool lescan**
   ```
   enquanto verdadeiro:
     executa hcitool lescan
     verifica resultados contra MACs autorizados
     se encontrou correspondência:
       escreve MAC no arquivo de detecção
       aguarda cooldown
     dorme 2 segundos
   ```

4. **Processo Principal**
   ```
   enquanto verdadeiro:
     verifica arquivo de detecção
     se arquivo existe:
       lê MAC
       envia pacote WoL
       remove arquivo de detecção
       aguarda cooldown
     dorme 1 segundo
   ```

#### Considerações Especiais

1. **Tratamento do DualShock 4**
   - O controle DualShock 4 pode usar diferentes endereços MAC:
     - Um MAC quando em modo de pareamento
     - Outro MAC quando ligado normalmente
   - Solução: Adicione ambos os MACs à lista autorizada
   - Use `hcidump -X` para identificar ambos os MACs
   - **Vantagem Principal**: A implementação Raspberry Pi pode detectar o controle em ambos os estados:
     - Quando está em modo de pareamento (usando hcitool inq)
     - Quando está ligado normalmente (usando hcidump)

2. **Detecção de Dispositivos Bluetooth Classic**
   - Alguns dispositivos só transmitem em modo de descoberta
   - Outros mantêm presença constante
   - Múltiplos métodos de detecção aumentam chances de detecção
   - `hcidump` pode pegar dispositivos mesmo quando não estão em modo de descoberta
   - Esta é uma vantagem significativa sobre a implementação ESP32, que só pode detectar dispositivos em modo de pareamento

3. **Otimização de Desempenho**
   - Cada método de detecção roda em paralelo
   - IPC baseado em arquivo minimiza overhead
   - Período de cooldown previne varredura excessiva
   - Gerenciamento de processos garante limpeza de recursos

4. **Tratamento de Erros**
   - Reinicialização automática de serviço em caso de falha
   - Monitoramento e recuperação de processos
   - Logging de todos os eventos significativos
   - Tratamento gracioso de desconexões de dispositivos

## Gerenciamento de Memória

Todas as implementações incluem considerações de gerenciamento de memória:
- Resultados de varredura BLE são limpos após processamento para liberar memória (nas implementações BLE e modo duplo)
- Variáveis temporárias são usadas eficientemente
- Operações de string são minimizadas
- Tamanhos de buffer são otimizados para as restrições de memória do ESP32

## Considerações de Energia

O consumo de energia varia entre as implementações:

### Implementação Apenas BLE
- Menor consumo de energia
- Varredura BLE eficiente com uso mínimo de rádio
- Recomendado para aplicações com bateria

### Implementação Modo Duplo
- Maior consumo de energia
- Executa rádios BLE e Bluetooth Classic
- Operações de varredura mais frequentes
- Processamento adicional para tratamento de protocolo duplo

### Implementação Apenas Bluetooth Classic
- Consumo médio de energia
- Maior que apenas BLE mas menor que modo duplo
- Varredura contínua aumenta uso de energia
- Mais eficiente que modo duplo para dispositivos apenas Classic

### Implementação Raspberry Pi
- Maior consumo de energia entre todas as implementações
- Executa múltiplos processos de detecção
- Monitoramento contínuo de tráfego Bluetooth
- Requer fonte de alimentação estável
- Não adequado para operação com bateria

## Considerações de Segurança

Esta implementação tem recursos mínimos de segurança:
- Autorização de dispositivo baseada apenas em endereço MAC
- Endereços MAC podem potencialmente ser falsificados
- Sem criptografia para o pacote Wake-on-LAN

Para segurança aprimorada, considere:
- Adicionar um mecanismo de handshake seguro
- Implementar pareamento de dispositivos
- Usar comunicação criptografada

### Segurança de Configuração

O projeto agora usa um arquivo de configuração separado (`config.h`) que é excluído do controle de versão via `.gitignore`. Isso ajuda a prevenir o commit acidental de informações sensíveis como:
- Credenciais Wi-Fi
- Informações de rede
- Endereços MAC de dispositivos

Ao compartilhar ou publicar este código, apenas o arquivo `config.example.h` é incluído, que contém valores de exemplo.

## Otimização de Desempenho

Várias otimizações são implementadas:
- Varredura ativa para detecção mais rápida de dispositivos BLE
- Período de cooldown para prevenir tráfego excessivo de rede
- Estratégia de varredura alternada para prevenir conflitos de recursos
- Término antecipado da varredura quando dispositivo autorizado é encontrado

## Sistema de Configuração

O projeto usa um sistema de configuração baseado em preprocessador que permite fácil personalização sem modificar os arquivos de código principais:

### Estrutura do Arquivo de Configuração

A configuração é gerenciada através de um arquivo de cabeçalho separado (`config.h`) que define macros de preprocessador:

```cpp
// Configurações Wi-Fi
#define WIFI_SSID "nome_da_rede"
#define WIFI_PASSWORD "senha"

// Configurações de rede
#define BROADCAST_IP_1 192
#define BROADCAST_IP_2 168
#define BROADCAST_IP_3 0
#define BROADCAST_IP_4 255

// Listas de dispositivos
#define ALLOWED_BLE_MACS { "mac1", "mac2", "mac3" }
#define ALLOWED_CLASSIC_MACS { "mac1", "mac2" }

// Configurações de tempo
#define BLE_SCAN_INTERVAL 1
#define CLASSIC_SCAN_CYCLES 5
#define WAKE_COOLDOWN 30000
```

### Benefícios Desta Abordagem

1. **Segurança**: Informações sensíveis são mantidas fora dos arquivos de código principais
2. **Controle de Versão**: O arquivo de configuração real é excluído via `.gitignore`
3. **Documentação**: O arquivo de configuração de exemplo serve como código auto-documentado
4. **Compatibilidade**: Funciona com o Arduino IDE sem requerer bibliotecas adicionais
5. **Flexibilidade**: Fácil modificar configurações sem mudar a funcionalidade principal

### Abordagens Alternativas Consideradas

Outras abordagens de configuração que poderiam ser implementadas em versões futuras:

1. **Armazenamento SPIFFS/LittleFS**: Armazenar configuração no sistema de arquivos do ESP32
2. **Armazenamento EEPROM**: Armazenar configuração em memória não-volátil
3. **Interface de Configuração Web**: Permitir configuração via navegador web

## Estendendo a Implementação

O código pode ser estendido com:
1. **Interface de Configuração Web**
   - Adicionar servidor web ESP32 para configuração
   - Armazenar configurações em memória não-volátil

2. **Suporte a Múltiplos Alvos**
   - Estender para suportar múltiplos PCs com diferentes endereços MAC
   - Associar dispositivos Bluetooth específicos com PCs específicos

3. **Gerenciamento Avançado de Energia**
   - Implementar deep sleep entre varreduras
   - Usar wake-up RTC para varredura periódica

4. **Integração com Automação Residencial**
   - Adicionar suporte MQTT
   - Implementar API REST para controle remoto
``` 