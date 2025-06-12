# Guia de Início Rápido

Este guia ajudará você a começar rapidamente com o projeto ESP32 Ligar PC via Bluetooth.

## Escolha Sua Implementação

O projeto oferece quatro implementações diferentes:

1. **Apenas BLE** (ESP32)
   - Melhor para controles modernos com suporte BLE
   - Menor consumo de energia
   - Tempo de resposta mais rápido

2. **Modo Duplo** (ESP32)
   - Suporta BLE e Bluetooth Classic
   - Bom para ambientes com dispositivos mistos
   - Consumo médio de energia

3. **Apenas Bluetooth Classic** (ESP32)
   - Melhor para controles mais antigos
   - Só detecta dispositivos em modo de pareamento
   - Consumo médio de energia

4. **Raspberry Pi**
   - Melhor para dispositivos Bluetooth Classic
   - Pode detectar dispositivos mesmo quando não estão em modo de pareamento
   - Mais robusto e flexível
   - Maior consumo de energia

> **Importante**: A implementação ESP32 só pode detectar dispositivos Bluetooth Classic (como o DualShock 4) quando estão em modo de pareamento. A implementação Raspberry Pi pode detectar estes dispositivos tanto em modo de pareamento quanto durante operação normal, tornando-a mais conveniente para uso diário.

## Início Rápido: Implementação ESP32

1. **Instale o Software Necessário**
   ```bash
   # Instale o Arduino IDE
   # Instale o suporte para ESP32
   # Instale as bibliotecas necessárias
   ```

2. **Configure o Projeto**
   ```bash
   cp config.example.h config.h
   # Edite config.h com suas configurações
   ```

3. **Faça Upload para o ESP32**
   - Abra o arquivo .ino desejado
   - Selecione sua placa
   - Clique em Upload

## Início Rápido: Implementação Raspberry Pi

1. **Instale o Software Necessário**
   ```bash
   sudo apt-get update
   sudo apt-get install bluez wakeonlan
   ```

2. **Execute o Script de Configuração**
   ```bash
   cd turn-on-pc-via-bluetooth-raspberry-pi
   sudo ./setup_monitor_service.sh
   ```

3. **Siga os Prompts**
   - Insira os endereços MAC dos dispositivos autorizados
   - Insira o endereço MAC do PC alvo
   - O script configurará e iniciará o serviço

## Encontrando Endereços MAC dos Dispositivos

### Para Implementação ESP32
1. Use um aplicativo de scanner Bluetooth no seu celular
2. Procure o dispositivo no scanner
3. Anote o endereço MAC mostrado

### Para Implementação Raspberry Pi
1. Execute `hcidump -X` no Raspberry Pi
2. Ligue seu dispositivo Bluetooth
3. Procure o endereço MAC na saída
4. Para DualShock 4, você pode ver dois MACs diferentes:
   - Um quando em modo de pareamento
   - Outro quando ligado normalmente
   - Adicione ambos à lista autorizada

## Testando a Configuração

1. **Habilite Wake-on-LAN**
   - Entre na BIOS/UEFI do seu PC
   - Habilite Wake-on-LAN
   - Salve e saia

2. **Teste a Conexão**
   - Ligue seu dispositivo Bluetooth
   - O ESP32/Raspberry Pi deve detectá-lo
   - Seu PC deve ligar

## Solução de Problemas

### Problemas com ESP32
- **Não detecta dispositivos**: Verifique os endereços MAC
- **Problemas de conexão**: Verifique as configurações Wi-Fi
- **Problemas de upload**: Segure o botão BOOT durante o upload

### Problemas com Raspberry Pi
- **Serviço não inicia**: Verifique o status com `sudo systemctl status monitorbt.service`
- **Dispositivo não detectado**: Execute `hcidump -X` para monitorar o tráfego Bluetooth
- **Problemas de permissão**: Certifique-se de que o script tem permissões de execução

## Próximos Passos

1. **Ajuste as Configurações**
   - Ajuste os intervalos de varredura
   - Modifique os períodos de cooldown
   - Adicione mais dispositivos autorizados

2. **Melhore a Segurança**
   - Revise a lista de dispositivos autorizados
   - Atualize as credenciais Wi-Fi
   - Considere medidas adicionais de segurança

3. **Otimize o Desempenho**
   - Monitore o consumo de energia
   - Ajuste os parâmetros de detecção
   - Ajuste os períodos de cooldown

## Precisa de Ajuda?

- Verifique a documentação completa em README.md
- Revise os detalhes técnicos em TECHNICAL_DETAILS.md
- Abra uma issue no GitHub
- Pergunte na seção de discussões 