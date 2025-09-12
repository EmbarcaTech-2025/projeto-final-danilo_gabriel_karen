#include "acelerometro_modulo.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#include "../config.h"

static i2c_inst_t *i2c_port = NULL;

static inline void i2c_write_reg(uint8_t reg, uint8_t val) {
    uint8_t b[2] = {reg, val};
    i2c_write_blocking(i2c_port, MPU_ADDR, b, 2, false);
}

static bool i2c_read_burst(uint8_t start_reg, uint8_t *buf, size_t len) {
    int ok = i2c_write_blocking(i2c_port, MPU_ADDR, &start_reg, 1, true);
    if (ok < 0) return false;
    ok = i2c_read_blocking(i2c_port, MPU_ADDR, buf, len, false);
    return ok >= 0;
}

void mpu6050_reset(void) {
    uint8_t b[2] = {0x6B, 0x80};
    i2c_write_blocking(i2c_port, MPU_ADDR, b, 2, false);
    sleep_ms(100);
    b[1] = 0x00;
    i2c_write_blocking(i2c_port, MPU_ADDR, b, 2, false);
    sleep_ms(10);

    i2c_write_reg(0x6B, 0x01);  // clock gyro X
    i2c_write_reg(0x1A, 0x02);  // DLPF ~94Hz
    i2c_write_reg(0x19, 3);     // Sample rate divider (250 Hz)
    i2c_write_reg(0x1B, 0x00);  // ±250 dps
    i2c_write_reg(0x1C, 0x00);  // ±2 g
}

void mpu6050_init() {
    i2c_port = ACELEROMETRO_PORT;
    i2c_init(i2c_port, ACELEROMETRO_BAUDRATE);
    gpio_set_function(ACELEROMETRO_SDA, GPIO_FUNC_I2C);
    gpio_set_function(ACELEROMETRO_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(ACELEROMETRO_SDA);
    gpio_pull_up(ACELEROMETRO_SCL);
    mpu6050_reset();
}

bool mpu6050_read(mpu6050_data_t *out) {
    uint8_t raw[14];
    if (!i2c_read_burst(0x3B, raw, 14)) return false;

    int16_t ax = (raw[0] << 8) | raw[1];
    int16_t ay = (raw[2] << 8) | raw[3];
    int16_t az = (raw[4] << 8) | raw[5];
    int16_t gx = (raw[8] << 8) | raw[9];
    int16_t gy = (raw[10] << 8) | raw[11];
    int16_t gz = (raw[12] << 8) | raw[13];

    out->ax = ax / ACC_SENS_2G;
    out->ay = ay / ACC_SENS_2G;
    out->az = az / ACC_SENS_2G;
    out->gx = gx / GYRO_SENS_250;
    out->gy = gy / GYRO_SENS_250;
    out->gz = gz / GYRO_SENS_250;

    return true;
}