# Sistema de Monitoramento para Idosos com Raspberry Pi Pico W

## 1. Problema a ser resolvido

Pessoas idosas, especialmente aquelas que vivem sozinhas ou possuem problemas de saúde, estão sujeitas a situações de risco como quedas, desorientação espacial e alterações fisiológicas graves (como baixa oxigenação no sangue ou arritmias). Muitas dessas situações não são percebidas a tempo por familiares ou cuidadores, podendo resultar em agravamentos de saúde ou até fatalidades.  

O projeto propõe o desenvolvimento de um dispositivo IoT vestível baseado na Raspberry Pi Pico W, que monitore a localização, os sinais vitais e a movimentação de idosos, gerando alertas automáticos em situações de risco.

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

- **RF03 - Detecção de quedas**
  - O sistema deve utilizar o sensor MPU6050 para detectar quedas com base na aceleração e rotação.
  - O sistema deve enviar um alerta automático caso uma queda seja detectada.

- **RF04 - Conectividade e envio de alertas**
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

---

