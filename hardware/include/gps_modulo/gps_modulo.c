#include "gps_modulo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../config.h"

// --- Funções privadas da biblioteca ---

/**
 * @brief Converte a posição do formato NMEA (DDMM.MMMM) para o formato decimal.
 */
static float convert_nmea_to_decimal(char *nmea_pos_str, char direction) {
    if (nmea_pos_str == NULL || strlen(nmea_pos_str) < 4) {
        return 0.0;
    }
    
    char *dot = strchr(nmea_pos_str, '.');
    if (dot == NULL) {
        return 0.0;
    }
    
    int minutes_len = strlen(dot);
    
    char degrees_str[5];
    strncpy(degrees_str, nmea_pos_str, strlen(nmea_pos_str) - minutes_len - 2);
    degrees_str[strlen(nmea_pos_str) - minutes_len - 2] = '\0';
    float degrees = atof(degrees_str);

    float minutes = atof(dot - 2);
    float decimal = degrees + minutes / 60.0;

    if (direction == 'S' || direction == 'W') {
        decimal *= -1.0;
    }
    
    return decimal;
}

/**
 * @brief Analisa a string NMEA GPGGA e preenche a estrutura gps_position_t.
 */
static void parse_gpgga(char *buffer, gps_position_t *position) {
    char *token;
    char *rest = buffer;
    position->is_valid = false;

    // Pula o primeiro token ($GPGGA)
    token = strtok_r(rest, ",", &rest);
    if (token == NULL || strcmp(token, "$GPGGA") != 0) {
        return; 
    }

    // Pula o tempo UTC
    token = strtok_r(rest, ",", &rest);
    if (token == NULL) return;

    // Latitude e sua direção
    char *lat_str = strtok_r(rest, ",", &rest);
    char *lat_dir_str = strtok_r(rest, ",", &rest);
    if (lat_str != NULL && lat_dir_str != NULL) {
        position->latitude = convert_nmea_to_decimal(lat_str, lat_dir_str[0]);
    } else return;

    // Longitude e sua direção
    char *lon_str = strtok_r(rest, ",", &rest);
    char *lon_dir_str = strtok_r(rest, ",", &rest);
    if (lon_str != NULL && lon_dir_str != NULL) {
        position->longitude = convert_nmea_to_decimal(lon_str, lon_dir_str[0]);
    } else return;
    
    // Status do GPS
    token = strtok_r(rest, ",", &rest);
    if (token != NULL) {
        int fix_quality = atoi(token);
        if (fix_quality > 0) {
            position->is_valid = true;
        }
    }
}

// --- Funções públicas da biblioteca ---

void gps_init() {
    uart_init(GPS_UART_ID, GPS_BAUDRATE);
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);
}

gps_position_t get_gps_location() {
    char nmea_buffer[MAX_NMEA_LEN];
    int buffer_index = 0;
    gps_position_t position = {0};
    printf("Parou aqui: 1\n");

    while (true) {
        if (uart_is_readable(GPS_UART_ID)) {
            char c = uart_getc(GPS_UART_ID);

            // printf("Parou aqui: 2\n");
            
            if (c == '\n' || c == '\r') {
                if (buffer_index > 0) {
                    nmea_buffer[buffer_index] = '\0';
                    parse_gpgga(nmea_buffer, &position);
                    // printf("Parou aqui: 3\n");
                    if (position.is_valid) {
                        return position;
                    }
                    buffer_index = 0;
                }
            } else if (buffer_index < MAX_NMEA_LEN - 1) {
                nmea_buffer[buffer_index++] = c;
                // printf("Parou aqui: Especial\n");
            }
            // printf("Parou aqui: 4\n");
        }
        // printf("Parou aqui: 5\n");
    }
    // printf("Parou aqui: 6\n");
}