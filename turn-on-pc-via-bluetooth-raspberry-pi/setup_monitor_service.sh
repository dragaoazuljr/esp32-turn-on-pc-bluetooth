#!/bin/bash

SCRIPT_PATH="/usr/local/bin/monitor_bt_inq.sh"
SERVICE_PATH="/etc/systemd/system/monitorbt.service"

echo "=== CONFIGURADOR DE MONITORAMENTO BLUETOOTH + WAKE-ON-LAN ==="
echo

# Lê MACs dos controles
CONTROLES_MAC=()
while true; do
    read -rp "Digite o MAC do controle Bluetooth (ou pressione Enter para finalizar): " mac
    [[ -z "$mac" ]] && break
    CONTROLES_MAC+=("\"$mac\"")
done

# Lê MAC do destino
read -rp "Digite o MAC do computador que será ligado via Wake-on-LAN: " DESTINO_MAC

# Gera script principal
echo "[INFO] Criando script de monitoramento em $SCRIPT_PATH..."

sudo tee "$SCRIPT_PATH" > /dev/null <<EOF
#!/bin/bash

# Lista de MACs dos controles Bluetooth
CONTROLES_MAC=(${CONTROLES_MAC[*]})

# MAC do PC destino
DESTINO_WOL="$DESTINO_MAC"

# Arquivo temporário para detecção
DETECT_FILE="/tmp/controle_detectado.tmp"
COOLDOWN=120

check_mac() {
  local mac=\$1
  for m in "\${CONTROLES_MAC[@]}"; do
    if [[ "\$mac" == "\$m" ]]; then
      return 0
    fi
  done
  return 1
}

loop_inq() {
  while true; do
    echo "[INFO] Executando hcitool inq..."
    RESULT=\$(hcitool inq --flush --length=10)
    for mac in "\${CONTROLES_MAC[@]}"; do
      if echo "\$RESULT" | grep -qi "\$mac"; then
        echo "[INQ] Controle \$mac detectado via inquiry"
        echo "\$mac" > "\$DETECT_FILE"
        sleep \$COOLDOWN
        break
      fi
    done
    sleep 2
  done
}

loop_hcidump() {
  sudo hcidump -X -i hci0 | while read -r line; do
    for mac in "\${CONTROLES_MAC[@]}"; do
      if echo "\$line" | grep -qi "\$mac"; then
        echo "[HCIDUMP] Controle \$mac detectado via hcidump"
        echo "\$mac" > "\$DETECT_FILE"
        sleep \$COOLDOWN
      fi
    done
  done
}

loop_hcitool_lescan() {
  while true; do
    echo "[INFO] Executando hcitool lescan..."
    RESULT=\$(sudo hcitool lescan)
    for mac in "\${CONTROLES_MAC[@]}"; do
      if echo "\$RESULT" | grep -qi "\$mac"; then
        echo "[LESCAN] Controle \$mac detectado via lescan"
        echo "\$mac" > "\$DETECT_FILE"
        sleep \$COOLDOWN
        break
      fi
    done
    sleep 2
  done
}

main_loop() {
  while true; do
    if [[ -f "\$DETECT_FILE" ]]; then
      MAC_DETECTADO=\$(cat "\$DETECT_FILE")
      echo "[MAIN] Detecção confirmada para \$MAC_DETECTADO, enviando WoL..."
      wakeonlan "\$DESTINO_WOL"
      rm -f "\$DETECT_FILE"
      sleep \$COOLDOWN
    fi
    sleep 1
  done
}

loop_inq &
PID_INQ=\$!
loop_hcidump &
PID_HCIDUMP=\$!
loop_hcitool_lescan &
PID_HCITOOL=\$!

main_loop

trap "kill \$PID_INQ \$PID_HCIDUMP \$PID_HCITOOL" EXIT
EOF

# Dá permissão de execução
sudo chmod +x "$SCRIPT_PATH"

# Cria serviço systemd
echo "[INFO] Criando serviço systemd em $SERVICE_PATH..."

sudo tee "$SERVICE_PATH" > /dev/null <<EOF
[Unit]
Description=Monitoramento de controle Bluetooth e Wake-on-LAN
After=bluetooth.target network-online.target

[Service]
Type=simple
ExecStart=$SCRIPT_PATH
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# Recarrega, habilita e reinicia o serviço
echo "[INFO] Recarregando e ativando serviço..."
sudo systemctl daemon-reexec
sudo systemctl daemon-reload
sudo systemctl enable monitorbt.service
sudo systemctl restart monitorbt.service

echo "[OK] Configuração finalizada. Serviço monitorbt.service está ativo!"
