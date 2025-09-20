#include "buzzer_modulo.h"

void buzzer_init(buzzer_t *buzzer, uint pin) {
    buzzer->buzzer_pin = pin;
    gpio_set_function(pin, GPIO_FUNC_PWM);
    buzzer->slice = pwm_gpio_to_slice_num(pin);

    pwm_set_wrap(buzzer->slice, 62499);   // Define período (freq. base)
    pwm_set_clkdiv(buzzer->slice, 1.0f);  // Clock direto
    pwm_set_enabled(buzzer->slice, true); // Ativa PWM
    pwm_set_gpio_level(pin, 0);           // Buzzer começa desligado
}

void buzzer_beep_ms(buzzer_t *buzzer, int duty, int ms) {
    pwm_set_gpio_level(buzzer->buzzer_pin, duty);
    sleep_ms(ms);
    pwm_set_gpio_level(buzzer->buzzer_pin, 0);
}
