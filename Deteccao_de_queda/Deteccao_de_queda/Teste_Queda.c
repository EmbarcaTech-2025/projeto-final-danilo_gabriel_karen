// Queda_curta_tethered.c — BitDogLab / RP2040 (Pico/Pico W) — TUDO EM 1 ARQUIVO
// I2C0: SDA=GP0, SCL=GP1 (conector branco)

#include <stdio.h>
#include <math.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// ===================== PINOS / I2C =====================
#define I2C_PORT        i2c0
#define I2C_SDA_PIN     0
#define I2C_SCL_PIN     1
#define MPU_ADDR        0x68          // mude para 0x69 se AD0=1

// ===================== CONFIG MPU ======================
// Amostragem mais alta para captar eventos curtíssimos
// 1 kHz base -> SMPLRT_DIV=3 => 250 Hz efetivos
#define MPU_SMPL_DIV    3             // 1kHz/(1+3)=250 Hz
// DLPF ~94 Hz: preserva picos de impacto mas reduz ruído
#define MPU_DLPF_CONF   0x02          // CONFIG=0x02 ~94 Hz
// Sensibilidades (±2g e ±250 dps)
#define ACC_SENS_2G     16384.0f      // LSB/g
#define GYRO_SENS_250   131.0f        // LSB/(°/s)

// ===================== FILTROS/HEURÍSTICAS =============
// EMA em |g| (suaviza só para "micro-free-fall")
#define EMA_ALPHA       0.35f

// Modo "queda curta com fio": duas rotas válidas
// ROTA A (micro-free-fall -> impacto)
#define MICRO_FF_G          0.60f     // |g|_filtrado < 0.60 g já vale como quase queda curta
#define MICRO_FF_MIN_MS     60        // por >= 60 ms (queda curtinha)
#define WAIT_IMPACT_MS      300       // tempo para esperar impacto depois do micro-free-fall
#define IMPACT_G_BRUTO      1.60f     // pico bruto de |g| > 1.60 g

// ROTA B (sem free-fall): Jerk + Giro (típico de puxão pelo fio)
#define DELTA_G_IMPACT      0.60f     // variação instantânea de |g| entre amostras
#define GYRO_DPS_THR        200.0f    // pico de velocidade angular (qualquer eixo) em °/s (curto)

// Confirmação leve após impacto
#define STABLE_G_LOW        0.75f
#define STABLE_G_HIGH       1.30f
#define ORIENT_DEG_MIN      20.0f

// Debug
#define PRINT_DBG_EVERY_MS  120
#define MAX_I2C_RETRY       2

typedef enum { FALL_IDLE=0, FALL_MICROFF, FALL_WAIT_IMPACT, FALL_CONFIRMED } fall_state_t;

// ===================== UTILS ===========================
static inline float vnorm3(float x, float y, float z){
    return sqrtf(x*x + y*y + z*z);
}
static inline float ema_update(float prev, float x, float alpha){
    return prev + alpha * (x - prev);
}
static void accel_to_angles(float ax, float ay, float az, float *pitch_deg, float *roll_deg){
    *pitch_deg = (180.0f/M_PI) * atanf(-ax / sqrtf(ay*ay + az*az + 1e-6f));
    *roll_deg  = (180.0f/M_PI) * atanf( ay / sqrtf(ax*ax + az*az + 1e-6f));
}

// ===================== I2C / MPU =======================
static inline void i2c_write_reg(uint8_t reg, uint8_t val){
    uint8_t b[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, b, 2, false);
}
static bool i2c_read_burst(uint8_t start_reg, uint8_t *buf, size_t len){
    int ok = i2c_write_blocking(I2C_PORT, MPU_ADDR, &start_reg, 1, true);
    if (ok < 0) return false;
    ok = i2c_read_blocking(I2C_PORT, MPU_ADDR, buf, len, false);
    return ok >= 0;
}

static void mpu_reset_and_config(void){
    // Reset
    uint8_t b[2] = {0x6B, 0x80};
    i2c_write_blocking(I2C_PORT, MPU_ADDR, b, 2, false);
    sleep_ms(100);
    // Acorda
    b[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU_ADDR, b, 2, false);
    sleep_ms(10);

    // Clock do gyro X, DLPF, SMPLRT_DIV, ranges
    i2c_write_reg(0x6B, 0x01);              // PWR_MGMT_1: clock gyro X
    i2c_write_reg(0x1A, MPU_DLPF_CONF);     // CONFIG: DLPF ~94 Hz
    i2c_write_reg(0x19, MPU_SMPL_DIV);      // 250 Hz
    i2c_write_reg(0x1B, 0x00);              // GYRO_CONFIG: ±250 dps
    i2c_write_reg(0x1C, 0x00);              // ACCEL_CONFIG: ±2 g
    sleep_ms(10);
}

// Lê 14 bytes (acc,temp,gyro)
static bool mpu_read_all14(int16_t *ax, int16_t *ay, int16_t *az,
                           int16_t *temp, int16_t *gx, int16_t *gy, int16_t *gz){
    uint8_t raw[14];
    if (!i2c_read_burst(0x3B, raw, 14)) return false;
    *ax   = (raw[0]  << 8) | raw[1];
    *ay   = (raw[2]  << 8) | raw[3];
    *az   = (raw[4]  << 8) | raw[5];
    *temp = (raw[6]  << 8) | raw[7];
    *gx   = (raw[8]  << 8) | raw[9];
    *gy   = (raw[10] << 8) | raw[11];
    *gz   = (raw[12] << 8) | raw[13];
    return true;
}

static bool mpu_read_accel_gyro(float *ax_g, float *ay_g, float *az_g,
                                float *gx_dps, float *gy_dps, float *gz_dps){
    for (int attempt = 0; attempt <= MAX_I2C_RETRY; attempt++){
        int16_t ax, ay, az, t, gx, gy, gz;
        if (mpu_read_all14(&ax,&ay,&az,&t,&gx,&gy,&gz)){
            *ax_g = ax / ACC_SENS_2G;
            *ay_g = ay / ACC_SENS_2G;
            *az_g = az / ACC_SENS_2G;
            *gx_dps = gx / GYRO_SENS_250;
            *gy_dps = gy / GYRO_SENS_250;
            *gz_dps = gz / GYRO_SENS_250;
            return true;
        }
        sleep_ms(2);
        if (attempt == MAX_I2C_RETRY){
            mpu_reset_and_config();
        }
    }
    return false;
}

// ===================== MAIN ===========================
int main(void){
    stdio_init_all();
    sleep_ms(2000); // tempo p/ USB enumerar

    // I2C0 @400 kHz em GP0/GP1
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Configura MPU
    mpu_reset_and_config();
    printf("MPU em I2C0 (SDA=GP0, SCL=GP1), addr=0x%02X, 250 Hz, DLPF~94Hz\n", MPU_ADDR);

    // Sanidade: 10 amostras
    for (int i = 0; i < 10; i++){
        float ax,ay,az,gx,gy,gz;
        if (!mpu_read_accel_gyro(&ax,&ay,&az,&gx,&gy,&gz)){ printf("Falha I2C init\n"); sleep_ms(20); continue; }
        printf("Init %02d: ax=%.2f ay=%.2f az=%.2f |g|=%.2f |gyro|=%.0f dps\n",
               i, ax, ay, az, vnorm3(ax,ay,az), vnorm3(gx,gy,gz));
        sleep_ms(40);
    }

    // FSM + filtros
    const uint32_t Ts_ms = 4;                // 250 Hz ~ 4 ms
    fall_state_t state = FALL_IDLE;
    uint32_t t_state = 0, t_dbg = 0;
    float g_lp = 1.0f;
    float prev_g = 1.0f;
    float last_pitch = 0.0f, last_roll = 0.0f;

    while (true){
        float ax, ay, az, gx, gy, gz;
        if (!mpu_read_accel_gyro(&ax,&ay,&az,&gx,&gy,&gz)){
            printf("Leitura I2C falhou, reconfig...\n");
            mpu_reset_and_config();
            sleep_ms(5);
            continue;
        }

        float g    = vnorm3(ax,ay,az);
        float dg   = fabsf(g - prev_g);
        prev_g     = g;
        g_lp       = ema_update(g_lp, g, EMA_ALPHA);

        float gyro_mag = vnorm3(gx,gy,gz);
        float pitch, roll; accel_to_angles(ax,ay,az,&pitch,&roll);

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - t_dbg >= PRINT_DBG_EVERY_MS){
            t_dbg = now;
            printf("DBG g=%.2f g_lp=%.2f dg=%.2f gyro=%.0f state=%d\n", g, g_lp, dg, gyro_mag, state);
        }

        switch (state){
            case FALL_IDLE:
                // ROTA A: micro-free-fall começa
                if (g_lp < MICRO_FF_G) {
                    state = FALL_MICROFF;
                    t_state = now;
                    last_pitch = pitch;
                    last_roll  = roll;
                }
                // ROTA B: sem free-fall — jerk + giro (puxão pelo fio)
                else if (dg > DELTA_G_IMPACT && gyro_mag > GYRO_DPS_THR) {
                    // Confirmação leve: estabilização ou mudança de orientação
                    bool stable = (g_lp >= STABLE_G_LOW && g_lp <= STABLE_G_HIGH);
                    bool orient = (fabsf(pitch - last_pitch) > ORIENT_DEG_MIN) ||
                                  (fabsf(roll  - last_roll ) > ORIENT_DEG_MIN);
                    printf("[ALERTA] Queda curta (jerk+gyro). g=%.2f dg=%.2f gyro=%.0f\n", g, dg, gyro_mag);
                    if (!stable && !orient) {
                        // se quiser, pode manter um flag e reavaliar nos próximos 100 ms
                    }
                    // Rearma rápido
                    sleep_ms(150);
                    state = FALL_IDLE;
                }
                break;

            case FALL_MICROFF:
                if (g_lp < MICRO_FF_G) {
                    if (now - t_state >= MICRO_FF_MIN_MS) {
                        state = FALL_WAIT_IMPACT;
                        t_state = now; // início da janela de impacto
                    }
                } else {
                    state = FALL_IDLE; // saiu do micro-free-fall cedo
                }
                break;

            case FALL_WAIT_IMPACT:
                // Aceita impacto por pico bruto OU por jerk+gyro
                if (g > IMPACT_G_BRUTO || (dg > DELTA_G_IMPACT && gyro_mag > GYRO_DPS_THR)) {
                    float dp = fabsf(pitch - last_pitch);
                    float dr = fabsf(roll  - last_roll);
                    bool orient_changed = (dp > ORIENT_DEG_MIN) || (dr > ORIENT_DEG_MIN);
                    bool stable = (g_lp >= STABLE_G_LOW && g_lp <= STABLE_G_HIGH);
                    printf("[ALERTA] Queda (microFF->impacto). g=%.2f dg=%.2f gyro=%.0f dP=%.1f dR=%.1f\n",
                           g, dg, gyro_mag, dp, dr);
                    // Rearme após curto tempo
                    sleep_ms(200);
                    state = FALL_IDLE;
                } else if (now - t_state > WAIT_IMPACT_MS) {
                    state = FALL_IDLE;
                }
                break;

            case FALL_CONFIRMED:
                sleep_ms(200);
                state = FALL_IDLE;
                break;
        }

        sleep_ms(Ts_ms); // ~250 Hz
    }
    return 0;
}
