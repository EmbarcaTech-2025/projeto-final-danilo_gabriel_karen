# Sistema de Monitoramento para Idosos com Raspberry Pi Pico W

## 1. Problema a ser resolvido

Pessoas idosas, especialmente aquelas que vivem sozinhas ou possuem problemas de saúde, estão sujeitas a situações de risco como quedas, desorientação espacial e alterações fisiológicas graves (como baixa oxigenação no sangue ou arritmias cardiacas). Muitas dessas situações não são percebidas a tempo por familiares ou cuidadores, podendo resultar em agravamentos de saúde ou até fatalidades.  

O projeto propõe o desenvolvimento de um dispositivo IoT vestível baseado na Raspberry Pi Pico W, para uso domiciliar, que monitore a localização, os sinais vitais e a movimentação de idosos, gerando alertas automáticos em situações de risco.

---

## 2. Requisitos Funcionais

- **RF01 - Monitoramento de localização**
  - O sistema deve captar coordenadas GPS periodicamente.
  - O sistema deve verificar se o idoso está dentro de um raio de segurança geográfico.
  - O sistema deve emitir um alerta (ex: via Wi-Fi) caso o raio de segurança seja ultrapassado.

- **RF02 - Medição de sinais vitais**
  - O sistema deve medir a frequência cardíaca e a saturação de oxigênio (SpO₂) usando o sensor MAX30100.
  - O sistema deve registrar os dados em tempo real para posterior análise de sono e estresse.
  - O sistema deve gerar alertas em caso de leituras anormais (ex: batimentos muito baixos/altos ou baixa oxigenação).

- **RF03 - Botão de Pânico**
  - O sistema deve reconhecer quando o usuário apertar um botão e enviar um alerta de emergência.

- **RF04 - Detecção de quedas**
  - O sistema deve utilizar o sensor MPU6050 para detectar quedas com base na aceleração e rotação.
  - O sistema deve enviar um alerta automático caso uma queda seja detectada.

- **RF05 - Conectividade e envio de alertas**
  - O sistema deve utilizar Wi-Fi para enviar notificações ou dados.

---

## 3. Requisitos Não Funcionais

- **RNF01 - Portabilidade**
  - O dispositivo deve ser compacto, leve e usável no corpo (ex: no bolso ou preso à roupa).

- **RNF02 - Baixo consumo de energia**
  - O sistema deve operar por longos períodos com uma bateria recarregável.
  - Deve haver gerenciamento de energia eficiente para prolongar a autonomia.

- **RNF03 - Tempo real**
  - Os sensores devem ser lidos e analisados em tempo real, garantindo reações rápidas a situações críticas.

- **RNF04 - Robustez e confiabilidade**
  - O sistema deve ser resistente a ruídos e pequenas falhas de leitura dos sensores.
  - Falhas temporárias na conexão Wi-Fi não devem travar o funcionamento principal.

- **RNF05 - Modularidade e escalabilidade**
  - O código deve ser modular para permitir manutenção e expansão futura.
  - Deve ser possível incluir outros sensores ou funcionalidades com facilidade.
 
- **RNF06 - Segurança elétrica**
  - O sistema deve garantir isolamento elétrico de pacientes.


## 🛠️ Lista de Materiais

| Componente                | Quantidade | Descrição                                                | Finalidade                                    |
|---------------------------|------------|------------------------------------------------------------|-----------------------------------------------|
| Raspberry Pi Pico W       | 1          | Microcontrolador com Wi-Fi integrado                      | Unidade principal de processamento e controle |
| Módulo GPS (NEO-6M)   | 1          | Módulo GPS com comunicação via UART                      | Captura de coordenadas geográficas            |
| Sensor MAX30100           | 1          | Sensor óptico para batimentos cardíacos e oxigenação     | Monitoramento da saúde                        |
| Sensor MPU6050            | 1          | Acelerômetro + Giroscópio (I2C)                          | Detecção de quedas através do movimento       |                    |
| Botão          | 1 ou mais  | Push button simples                                      | Entrada manual para testes                    |
| Buzzer          | 1   | Buzzer piezoelétrico                                      | Alarme sonoro                   |
| Display OLED SSD1306          | 1   | Exibir dados e alarmes                                      | Interface intuitiva para o usuário                  |
| Módulo ECG (AD8232)      | 1   | Processar os sinais de ECG captados pelos eletrodos                                      | Analisar o estado de saúde cardíaca             |
| Eletrodos com cabos     | 3   | Captar os sinais cardíacos de sístole e diástole                                      | Captação dos sinais elétricos do coração             |

## 4. Proposta de Arquitetura do Sistema

A proposta de arquitetura do sistema inclui três visões principais:

1. **Diagrama de Hardware** — Representa a disposição física e conexões entre os componentes do sistema.
   
   ![Diagrama de Hardware](imagens/DiagramaHardware_MonitorSinaisVitais.png)

2. **Blocos Funcionais** — Mostra os módulos lógicos e como eles interagem para realizar as funções do sistema.

    ![Diagrama de Blocos Funcionais](imagens/blocos_funcionais.png)

3. **Fluxograma do Software** — Representa o fluxo das operações realizadas pelo firmware, desde a inicialização até o envio de dados e alertas.
   ![Fluxograma de Software](imagens/Fluxograma_Software.jpg)



