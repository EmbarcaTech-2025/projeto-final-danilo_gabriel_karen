#ifndef FALL_DETECTOR_H
#define FALL_DETECTOR_H

#include "acelerometro_modulo.h"
#include <stdbool.h>

typedef enum {
    FALL_IDLE = 0,
    FALL_MICROFF,
    FALL_WAIT_IMPACT,
    FALL_CONFIRMED
} fall_state_t;

void fall_detector_init(void);
bool fall_detector_update(const mpu6050_data_t *data);

#endif
