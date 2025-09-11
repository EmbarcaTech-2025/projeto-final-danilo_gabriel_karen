#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string.h>
// Free RTOS includes
#include "FreeRTOS.h" // Inclui as definições do FreeRTOS
#include "task.h"   // Inclui as definições para tarefas do FreeRTOS

// Includes driver
#include "include/gps_modulo/gps_modulo.h"
#include "include/wifi_modulo/wifi_modulo.h"
#include "include/mqtt_modulo/mqtt_modulo.h"
#include "config.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);
    gps_init();
    wifi_connect(WIFI_SSID, WIFI_PASS);

    mqtt_setup(MQTT_USUARIO, MQTT_IP_BROKER, NULL, NULL);

    sleep_ms(5000);

    char msg[100];
    sprintf(msg, "{\"lat\":%.6f,\"lon\":%.6f}", 32, 54);

    mqtt_comm_publish("usuario/gps", (uint8_t*)msg, strlen(msg));



    // mqtt_comm_publish("usuario/gps", mensagem, strlen(mensagem));

    printf("Aguardando localizacao valida do GPS...\n");

    
    while (true) {
        gps_position_t current_pos = get_gps_location();
        
        if (current_pos.is_valid) {
            printf("Posicao obtida!\n");
            printf("Latitude: %.6f\n", current_pos.latitude);
            printf("Longitude: %.6f\n", current_pos.longitude);
        }

        sleep_ms(5000); 
    }
    
    return 0;
}

