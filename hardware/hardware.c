#include <stdio.h>
#include "pico/stdlib.h"
// Free RTOS includes
#include "FreeRTOS.h" // Inclui as definições do FreeRTOS
#include "task.h"   // Inclui as definições para tarefas do FreeRTOS

// Includes driver
#include "include/gps_modulo/gps_modulo.h"




int main()
{
    stdio_init_all();

    while (true) {
        inicializa_gps();
        sleep_ms(1000);
    }
}
