#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string.h>
// includes Free RTOS 
#include "FreeRTOS.h" // Inclui as definições do FreeRTOS
#include "task.h"   // Inclui as definições para tarefas do FreeRTOS
// Includes driver
#include "include/gps_modulo/gps_modulo.h"
#include "include/wifi_modulo/wifi_modulo.h"
#include "include/mqtt_modulo/mqtt_modulo.h"
#include "include/acelerometro_modulo/acelerometro_modulo.h"
#include "include/acelerometro_modulo/detector_queda.h"
// Configurações
#include "config.h"

// Função de envio de GPS
void send_gps_position() {
    printf("Entrou na função blocante\n");
    gps_position_t posicao_atual = get_gps_location();
    printf("Saiu da função da blocante\n");

    if (!posicao_atual.is_valid) {
        printf("GPS inválido, não enviando.\n");
        return;
    }

    char payload[128];

    int len = snprintf(payload, sizeof(payload),
                       "{\"usuarioId\":%d,\"latitude\":%.6f,\"longitude\":%.6f}",
                       ID_USUARIO, posicao_atual.latitude, posicao_atual.longitude);

    if (len < 0 || len >= sizeof(payload)) {
        printf("Erro ao formatar payload JSON.\n");
        return;
    }

    mqtt_comm_publish(GPS_ROUTE, (const uint8_t*)payload, strlen(payload));

    printf("Mensagem enviada com sucesso\n");
}

void gps_task(void* pvParameters) {

    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        send_gps_position();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(GPS_SEND_INTERVAL_MS));
    }
}

int main() {
    // Setup de configuração
    stdio_init_all();
    wifi_connect(WIFI_SSID, WIFI_PASS);
    printf("WIFI\n");
    sleep_ms(2000);
    gps_init();
    printf("GPS\n");
    sleep_ms(3000);
    mqtt_setup(MQTT_USUARIO, MQTT_IP_BROKER, NULL, NULL);    
    printf("Mensagem mqtt");
    sleep_ms(2000);


    printf("passou dos teste\n");
    
    xTaskCreate(gps_task, "GPS_Task", 2048, NULL, 2, NULL);
    
    vTaskStartScheduler();

    while (true) {
            sleep_ms(1000);
            printf("Deu no erro no vTask\n"); }

    return 0;
}

