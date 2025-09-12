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
#include "config.h"

int main() {
    stdio_init_all();
    
    sleep_ms(2000);

    mpu6050_init();
    fall_detector_init();
    
    mpu6050_data_t d;

    while (1)
    {
        if (mpu6050_read(&d)) {
            if (fall_detector_update(&d)) {
                printf("QUEDA DETECTADA\n");
                sleep_ms(2000);
            }
        }
        
        sleep_ms(10);

        printf("TESTE DE ACELERAÇÃO: %d\n", d.ax);
    }

    return 0;
}

