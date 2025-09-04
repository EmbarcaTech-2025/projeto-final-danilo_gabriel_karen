# Sensor Oxímetro e Batimentos Cardíacos

### Testes com o Sensor MAX3010x

Nesta etapa foram realizados os primeiros testes com o sensor **MAX3010x**, responsável pela medição da frequência cardíaca e da saturação de oxigênio (SpO₂). O sensor **MAX3010x** foi envolvido por uma capa protetora a fim de reduzir a incidência de luz externa no experimento.
Durante os experimentos iniciais, foi possível validar a comunicação I²C com a Raspberry Pi Pico W e capturar sinais fisiológicos básicos.  

O comportamento observado indicou leituras de frequência cardíaca consistentes. Já a medição de SpO₂ apresentou melhor resposta quando o dedo foi levemente afastado do sensor, sugerindo necessidade de ajustes no posicionamento e na calibração do algoritmo de filtragem.

No seguinte link é possível verificar o teste com o sensor:

https://youtube.com/shorts/pUu1NJMdmvI?feature=share


### Os códigos:
> `oxi_bat_sensor_raw.c`
> Recebe os valores de IR e RED lidos pelo sensor e os exibe via Serial Monitor.

> `oximeter_heart_rate.c`
> Recebe os valores de IR e RED lidos pelo sensor, os processa em valores de SpO2 (em %) e BPM, e os exibe via Serial Monitor.
