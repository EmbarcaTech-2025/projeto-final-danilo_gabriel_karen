#ifndef ACELEROMETRO_MODULO_H
#define ACELEROMETRO_MODULO_H

#include <stdbool.h>
#include <stdint.h>
#include "hardware/i2c.h"

// Config padrão
#define MPU_ADDR        0x68
#define ACC_SENS_2G     16384.0f
#define GYRO_SENS_250   131.0f

typedef struct {
    float ax, ay, az;    
    float gx, gy, gz;    
} mpu6050_data_t;

void mpu6050_init();
bool mpu6050_read(mpu6050_data_t *out);
void mpu6050_reset(void);

#endif
