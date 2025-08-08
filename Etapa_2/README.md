# Etapa 2 – Proposta de Arquitetura do Sistema

## 1) Diagrama de Hardware

O diagrama de hardware ilustra a disposição física e as conexões elétricas entre os componentes do Sistema de Monitoramento.
O microcontrolador central é o Raspberry Pi Pico W, responsável por gerenciar os sensores, processar dados e realizar a comunicação. Os principais módulos e conexões são:

- **GPS GY-NEO-6MV2** — Fornece dados de localização via interface UART (pinos GP0 e GP1).

- **Botão A** — Entrada digital para funções de controle do sistema. (pino GP 5)

- **Buzzer** — Saída sonora para alertas, ligado ao pino GP9. (pino GP 10)

- **Display OLED SSD1306** — Interface I2C para exibir informações ao usuário (pinos GP14 e GP15).

- **Sensor MPU6050** — Acelerômetro e giroscópio para detecção de movimentos e inclinação 

- **Módulo MAX30100** — Sensor óptico para leitura de frequência cardíaca e oximetria de pulso 

- **Módulo AD8232** — Circuito de ECG para monitoramento de sinais cardíacos, conectado aos pinos analógicos e digitais específicos para leitura dos sinais de eletrocardiograma. (saída no pino GP 31)

- **Conexões de Eletrodos (RA, LA, RL)** — Eletrodos RL (perna direita), LA (braço esquerdo) e RL (perna direita), posicionados no corpo para captação do sinal cardíaco. 
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
1. **Inicialização**: configura GPIO/I²C/UART/ADC e autotestes.  
2. **Conectividade**: Wi‑Fi (TLS) e sincronização de tempo.  
3. **Loop**: coleta (ECG/PPG/IMU/GPS) → pré‑processamento → métricas/eventos.  
4. **Ações**: exibe no OLED, aciona buzzer, empacota e envia (MQTT/HTTP).  
5. **Economia de energia**: sleep/duty‑cycle e retomada do loop.
![Fluxograma de Software](Fluxograma_Software.jpg)
