#include "detector_queda.h"
#include "pico/stdlib.h"
#include <math.h>
#include <stdio.h>

// --- Config e thresholds ---
#define EMA_ALPHA       0.35f
#define MICRO_FF_G      0.60f
#define MICRO_FF_MIN_MS 60
#define WAIT_IMPACT_MS  300
#define IMPACT_G_BRUTO  1.60f
#define DELTA_G_IMPACT  0.60f
#define GYRO_DPS_THR    200.0f
#define STABLE_G_LOW    0.75f
#define STABLE_G_HIGH   1.30f
#define ORIENT_DEG_MIN  20.0f

static inline float vnorm3(float x, float y, float z) {
    return sqrtf(x*x + y*y + z*z);
}

static inline float ema_update(float prev, float x, float alpha) {
    return prev + alpha * (x - prev);
}

static fall_state_t state = FALL_IDLE;
static float g_lp = 1.0f, prev_g = 1.0f;
static uint32_t t_state = 0;

void fall_detector_init(void) {
    state = FALL_IDLE;
    g_lp = 1.0f;
    prev_g = 1.0f;
    t_state = 0;
}

bool fall_detector_update(const mpu6050_data_t *d) {
    float g = vnorm3(d->ax, d->ay, d->az);
    float dg = fabsf(g - prev_g);
    prev_g = g;
    g_lp = ema_update(g_lp, g, EMA_ALPHA);
    float gyro_mag = vnorm3(d->gx, d->gy, d->gz);

    uint32_t now = to_ms_since_boot(get_absolute_time());

    switch (state) {
        case FALL_IDLE:
            if (g_lp < MICRO_FF_G) {
                state = FALL_MICROFF;
                t_state = now;
            } else if (dg > DELTA_G_IMPACT && gyro_mag > GYRO_DPS_THR) {
                printf("[ALERTA] Queda curta (jerk+gyro)\n");
                return true;
            }
            break;

        case FALL_MICROFF:
            if (g_lp < MICRO_FF_G) {
                if (now - t_state >= MICRO_FF_MIN_MS) {
                    state = FALL_WAIT_IMPACT;
                    t_state = now;
                }
            } else {
                state = FALL_IDLE;
            }
            break;

        case FALL_WAIT_IMPACT:
            if (g > IMPACT_G_BRUTO || (dg > DELTA_G_IMPACT && gyro_mag > GYRO_DPS_THR)) {
                printf("[ALERTA] Queda (microFF->impacto)\n");
                state = FALL_IDLE;
                return true;
            } else if (now - t_state > WAIT_IMPACT_MS) {
                state = FALL_IDLE;
            }
            break;

        default:
            state = FALL_IDLE;
    }
    return false;
}
