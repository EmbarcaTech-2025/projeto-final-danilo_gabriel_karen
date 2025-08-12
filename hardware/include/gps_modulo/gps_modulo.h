#ifndef GPS_MODULO_H
#define GPS_MODULO_H

#include "pico/stdlib.h"
#include "hardware/uart.h"

// Estrutura para armazenar a latitude e longitude
typedef struct {
    float latitude;
    float longitude;
    bool is_valid;
} gps_position_t;

/**
 * @brief Inicializa a UART para comunicação com o módulo GPS.
 */
void gps_init();

/**
 * @brief Obtém a localização do GPS de forma bloqueante.
 * * Esta função espera até receber uma string GPGGA válida e processá-la.
 * * @return gps_position_t Uma estrutura contendo a latitude, longitude e
 * um flag de validade (true se a posição for válida).
 */
gps_position_t get_gps_location();

#endif