#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
// Free RTOS includes
#include "FreeRTOS.h" // Inclui as definições do FreeRTOS
#include "task.h"   // Inclui as definições para tarefas do FreeRTOS

// Includes driver
#include "include/gps_modulo/gps_modulo.h"
#include "config.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);
    gps_init(); // Inicializa a UART do GPS

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
