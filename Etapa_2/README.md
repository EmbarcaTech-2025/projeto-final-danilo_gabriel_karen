# Etapa 2 – Proposta de Arquitetura do Sistema

## 1) Diagrama de Hardware

O diagrama de hardware ilustra a disposição física e as conexões elétricas entre os componentes do Monitor de Sinais Vitais.
O microcontrolador central é o Raspberry Pi Pico W, responsável por gerenciar os sensores, processar dados e realizar a comunicação. Os principais módulos e conexões são:

GPS GY-NEO-6MV2 — Fornece dados de localização via interface UART (pinos GP0 e GP1).

Botão A — Entrada digital para funções de controle do sistema.

Buzzer — Saída sonora para alertas, ligado ao pino GP9.

Display OLED SSD1306 — Interface I2C para exibir informações ao usuário (pinos GP14 e GP15).

Sensor MPU6050 — Acelerômetro e giroscópio para detecção de movimentos e inclinação (I2C nos pinos GP26 e GP27).

Módulo MAX30100 — Sensor óptico para leitura de frequência cardíaca e oximetria de pulso (I2C nos pinos GP26 e GP27).

Módulo AD8232 — Circuito de ECG para monitoramento de sinais cardíacos, conectado aos pinos analógicos e digitais específicos para leitura dos sinais de eletrocardiograma.

Conexões de Eletrodos (RA, LA, RL) — Eletrodos posicionados no corpo para captação do sinal cardíaco.
![Diagrama de Hardware](DiagramaHardware_MonitorSinaisVitais.png)

---

## 2) Diagrama de Blocos Funcionais
Mostra os módulos lógicos e como eles interagem:
- **Aquisição de sinais**: ECG (AD8232), SpO₂/FC (MAX30100), IMU (MPU6050), GPS e botão.
- **Processamento de borda**: filtros, extração de métricas, detecção de eventos e máquina de estados.
- **Comunicação & persistência**: empacotamento (JSON/CBOR), envio MQTT/HTTP (TLS), buffer e histórico (TSDB).
- **Interface do usuário**: OLED, buzzer, botão e App/PWA no celular.
- **Energia & segurança**: duty-cycle/sleep, watchdog e criptografia.

![Diagrama de Blocos Funcionais](blocos_funcionais.png)

---

## 3) Fluxograma do Software
Fluxo das operações do firmware, desde a inicialização até o envio de dados/alertas:
1. Boot → inicialização de hardware e autotestes.  
2. Conexão Wi‑Fi (TLS) e sincronização de tempo.  
3. Loop: coleta (ECG/PPG/IMU/GPS) → pré-processamento → métricas/eventos.  
4. Empacotamento e envio (MQTT/HTTP), registro local, alarmes e interface.  
5. Sleep/duty‑cycle para economia de energia.

![Fluxograma de Software](Fluxograma_Software.jpg)
