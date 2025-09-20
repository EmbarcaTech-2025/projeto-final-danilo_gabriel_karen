#ifndef BUZZER_H
#define BUZZER_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Estrutura de configuração do buzzer
typedef struct {
    uint buzzer_pin;   // Pino do buzzer
    uint slice;        // Canal PWM usado
} buzzer_t;

/**
 * @brief Inicializa o buzzer em um pino específico
 * 
 * @param buzzer Estrutura do buzzer
 * @param pin Número do GPIO onde o buzzer está conectado
 */
void buzzer_init(buzzer_t *buzzer, uint pin);

/**
 * @brief Emite um beep por um tempo em ms
 * 
 * @param buzzer Estrutura do buzzer
 * @param duty Duty cycle (0..wrap definido, ex: 62499)
 * @param ms Duração em milissegundos
 */
void buzzer_beep_ms(buzzer_t *buzzer, int duty, int ms);

#endif // BUZZER_H
