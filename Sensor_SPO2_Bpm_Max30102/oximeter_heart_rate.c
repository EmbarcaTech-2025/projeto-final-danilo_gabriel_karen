/*
  MAX30102 - HR & SpO2 (contato direto corrigido + AGC)
  - Janela ~3 s @ 100 Hz (SAMPLE_SIZE=300)
  - Suavização (média móvel) para detecção de picos
  - Detector com histerese + refratário dinâmico
  - BPM pela mediana dos últimos RR (3..5 intervalos)
  - SpO2 por ratio-of-ratios com AC/DC (RMS do detrendido)
  - Consistência do R em 3 sub-janelas (limite mais tolerante)
  - "Hold + EMA" do SpO2 para evitar flicker
  - AGC dos LEDs visando IR_DC ~ 50–80k (adequado p/ contato)
  - FIFO averaging = 8 (melhor SNR sem achatar demais o pulso)

  Placa: RP2040 (Pico/BitDogLab) — I2C0 nos pinos 0/1
*/

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

//----------------- FLAGS -----------------
#define DEBUG     0  // 1: prints de diagnóstico (IR_DC, perf, R, LED)
#define MECH_HINT 1  // 1: dica automática se perfusão baixa persistir

//----------------- I2C / MAX30102 -----------------
#define I2C_PORT i2c0
#define I2C_SDA  0
#define I2C_SCL  1
#define MAX30102_ADDR 0x57

// Registradores
#define REG_INTR_STATUS_1 0x00
#define REG_INTR_ENABLE_1 0x02
#define REG_FIFO_WR_PTR   0x04
#define REG_FIFO_OVF_CNT  0x05
#define REG_FIFO_RD_PTR   0x06
#define REG_FIFO_DATA     0x07
#define REG_FIFO_CONFIG   0x08
#define REG_MODE_CONFIG   0x09
#define REG_SPO2_CONFIG   0x0A
#define REG_LED1_PA       0x0C    // RED
#define REG_LED2_PA       0x0D    // IR
#define REG_PART_ID       0xFF    // 0x15 para MAX30102

//----------------- Parâmetros de processamento -----------------
#define FS_HZ         100.0f
#define SAMPLE_SIZE   300       // ~3 s
#define MA_WIN        5         // média móvel p/ IR (picos)
#define MAX_PEAKS     32

// Alvos/limiares para SpO2 (ajustados p/ contato)
#define DC_MIN    20000.f
#define DC_MAX   200000.f
#define PERF_MIN   0.0015f      // perfusão mínima (AC/DC)

// AGC: alvo e faixa "deadband" (mais baixos p/ contato direto)
#define IR_DC_TARGET_LOW   50000.f
#define IR_DC_TARGET_HIGH  80000.f

//----------------- Buffers -----------------
static uint32_t red_buffer[SAMPLE_SIZE];
static uint32_t ir_buffer [SAMPLE_SIZE];

//----------------- I2C helpers -----------------
static void config_i2c(void) {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

static inline void i2c_write8(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(I2C_PORT, MAX30102_ADDR, buf, 2, false);
}

static inline uint8_t i2c_read8(uint8_t reg) {
    uint8_t v;
    i2c_write_blocking(I2C_PORT, MAX30102_ADDR, &reg, 1, true);
    i2c_read_blocking (I2C_PORT, MAX30102_ADDR, &v, 1, false);
    return v;
}

//----------------- LEDs / AGC -----------------
static uint8_t led_ir  = 0x24;  // start
static uint8_t led_red = 0x24;

static inline void set_leds(uint8_t red, uint8_t ir){
    i2c_write8(REG_LED1_PA, red);
    i2c_write8(REG_LED2_PA, ir);
}

static inline void agc_update(float ir_dc){
    // Passos pequenos para evitar aquecimento e overshoot
    if (ir_dc < IR_DC_TARGET_LOW && led_ir < 0x50){
        if (led_ir <= 0x4C) { led_ir += 0x04; led_red += 0x04; }
        else                { led_ir += 0x02; led_red += 0x02; }
        set_leds(led_red, led_ir);
    } else if (ir_dc > IR_DC_TARGET_HIGH && led_ir > 0x18){
        if (led_ir >= 0x1C) { led_ir -= 0x04; led_red -= 0x04; }
        else                { led_ir -= 0x02; led_red -= 0x02; }
        set_leds(led_red, led_ir);
    }
#if DEBUG
    printf("[agc] IR_DC=%.0f  LED_IR=0x%02X  LED_RED=0x%02X\n", ir_dc, led_ir, led_red);
#endif
}

//----------------- MAX30102 init / read -----------------
static void max30102_init(void) {
    // reset
    i2c_write8(REG_MODE_CONFIG, 0x40);
    sleep_ms(100);

    // zera FIFO
    i2c_write8(REG_FIFO_WR_PTR, 0x00);
    i2c_write8(REG_FIFO_OVF_CNT, 0x00);
    i2c_write8(REG_FIFO_RD_PTR, 0x00);

    // FIFO: avg = 8 (0b011), rollover=1, almost_full=17
    i2c_write8(REG_FIFO_CONFIG, (0b011<<5) | (1<<4) | 0x11);

    // SpO2: ADC range = 4096nA (01), SR=100Hz (011), PW=411us 18-bit (11)
    i2c_write8(REG_SPO2_CONFIG, (0b01<<5) | (0b011<<2) | 0b11);

    // Corrente dos LEDs (ponto de partida)
    set_leds(led_red, led_ir);

    // modo SpO2 (RED + IR)
    i2c_write8(REG_MODE_CONFIG, 0x03);
}

static bool max30102_read_sample(uint32_t *red, uint32_t *ir) {
    uint8_t wr = i2c_read8(REG_FIFO_WR_PTR);
    uint8_t rd = i2c_read8(REG_FIFO_RD_PTR);
    if (wr == rd) return false;

    uint8_t data[6];
    uint8_t reg = REG_FIFO_DATA;
    i2c_write_blocking(I2C_PORT, MAX30102_ADDR, &reg, 1, true);
    i2c_read_blocking (I2C_PORT, MAX30102_ADDR, data, 6, false);

    *red = ((uint32_t)data[0]<<16 | data[1]<<8 | data[2]) & 0x3FFFF;
    *ir  = ((uint32_t)data[3]<<16 | data[4]<<8 | data[5]) & 0x3FFFF;
    return true;
}

//----------------- Processamento de Sinal -----------------
static void moving_average_u32(const uint32_t *in, float *out, int n, int win){
    float acc = 0.f;
    for(int i=0;i<n;i++){
        acc += (float)in[i];
        if (i>=win) acc -= (float)in[i-win];
        out[i] = (i<win-1) ? (acc/(float)(i+1)) : (acc/(float)win);
    }
}

// AC/DC por RMS do "detrendido": remove drift de baixa frequência
static void acdc_hp_rms(const uint32_t *sig, int n, int win_dc, float *ac_rms, float *dc_mean){
    static float ma[SAMPLE_SIZE];
    double acc = 0.0;
    for(int i=0;i<n;i++){
        acc += (double)sig[i];
        if (i>=win_dc) acc -= (double)sig[i-win_dc];
        ma[i] = (i<win_dc-1) ? (float)(acc/(double)(i+1)) : (float)(acc/(double)win_dc);
    }
    double sum2 = 0.0, sumdc = 0.0;
    for(int i=0;i<n;i++){
        float d = (float)sig[i] - ma[i];
        sum2  += (double)d*d;
        sumdc += (double)ma[i];
    }
    *ac_rms  = (float)sqrt(sum2/(double)n);
    *dc_mean = (float)(sumdc/(double)n);
}

//---- Detector com histerese + refratário dinâmico ----
typedef struct {
    float mu, sigma;
    float th_high, th_low;
    int   refractory;
    int   armed;
    float last_x;
} peakdet2_t;

static void peakdet2_init(peakdet2_t *st){
    st->mu=0.f; st->sigma=1.f; st->th_high=0.f; st->th_low=0.f;
    st->refractory=0; st->armed=0; st->last_x=0.f;
}

static inline void ema_update(float x, float *mu, float *sigma){
    const float a_mu = 0.02f, a_s = 0.02f;
    float d = x - *mu;
    *mu += a_mu * d;
    float var = (*sigma)*(*sigma);
    var = (1.f - a_s)*var + a_s * d*d;
    *sigma = sqrtf(var);
}

static int peakdet2_step(peakdet2_t *st, float x, float k, int refract_samples){
    if (st->refractory > 0){ st->refractory--; st->last_x = x; return 0; }

    ema_update(x, &st->mu, &st->sigma);
    st->th_high = st->mu + k * st->sigma;
    st->th_low  = st->mu + (k - 0.3f) * st->sigma;

    int peak = 0;
    if (!st->armed){
        if (x > st->th_high) st->armed = 1;           // cruzou para cima
    } else {
        if (st->last_x > x && x < st->th_low){        // começou a cair e passou TH_LOW
            peak = 1;
            st->refractory = refract_samples;
            st->armed = 0;
        }
    }
    st->last_x = x;
    return peak;
}

// BPM pela mediana dos últimos RR (3..5 intervalos)
static float bpm_from_last_rr_median(const int *p, int n, float fs){
    if (n < 3) return 0.f;
    int m = n-1; if (m>5) m=5;
    float rr[5];
    for(int j=0;j<m;j++){
        int i = n-1-j;
        rr[j] = (float)(p[i]-p[i-1]) / fs;
    }
    // ordena rr (insertion sort)
    for(int i=0;i<m;i++) for(int j=i+1;j<m;j++) if (rr[j]<rr[i]) { float t=rr[i]; rr[i]=rr[j]; rr[j]=t; }
    float med = rr[m/2];
    if (med <= 0.f) return 0.f;
    float bpm = 60.f/med;
    if (bpm < 45.f || bpm > 180.f) return 0.f;  // clamp fisiológico inicial
    return bpm;
}

//----------------- SpO2 (ratio-of-ratios) -----------------
static bool calculate_spo2_rr(const uint32_t *red, const uint32_t *ir, int n, float *spo2_out){
    float red_ac, ir_ac, red_dc, ir_dc;

    // detrending window ~0.25 s (ajustável: 20..40 amostras)
    const int WIN_DC = 25;
    acdc_hp_rms(red, n, WIN_DC, &red_ac, &red_dc);
    acdc_hp_rms(ir , n, WIN_DC, &ir_ac , &ir_dc);

    // AGC (atua para próxima janela)
    agc_update(ir_dc);

    // validações básicas (tolerantes p/ contato)
    if (ir_dc < DC_MIN || red_dc < DC_MIN) {
#if MECH_HINT
        static int low_dc_streak = 0;
        if (++low_dc_streak >= 2) {
            printf("[hint] Pouca luz detectada. Ajuste o dedo e evite luz ambiente.\n");
        }
#endif
        return false;
    }
#if MECH_HINT
    else {
        // reset quando OK
        static int low_dc_streak = 0; low_dc_streak = 0;
    }
#endif
    if (ir_dc > DC_MAX || red_dc > DC_MAX) {
#if MECH_HINT
        printf("[hint] DC muito alto (%.0f). Solte um pouco o dedo (pressão).\n", ir_dc);
#endif
        return false;
    }

    float perf_ir  = ir_ac  / ir_dc;
    float perf_red = red_ac / red_dc;
    if (perf_ir < PERF_MIN || perf_red < PERF_MIN) {
#if MECH_HINT
        static int low_perf_streak = 0;
        if (++low_perf_streak >= 2) {
            printf("[hint] Perfusão baixa (AC/DC). Solte um pouco o dedo e mantenha contato leve.\n");
        }
#else
        (void)perf_ir; (void)perf_red;
#endif
        return false;
    }
#if MECH_HINT
    else {
        static int low_perf_streak = 0; low_perf_streak = 0;
    }
#endif

    // R global
    float R = (red_ac / red_dc) / (ir_ac / ir_dc);

    // Consistência do R por segmentos (3 blocos) — limite mais tolerante
    int m = n/3;
    float Rseg[3];
    for(int s=0;s<3;s++){
        int len = (s==2 ? n-2*m : m);
        float rac, rdc, iac, idc;
        acdc_hp_rms(red + s*m, len, WIN_DC, &rac, &rdc);
        acdc_hp_rms(ir  + s*m, len, WIN_DC, &iac, &idc);
        float pr = rac/rdc, pi = iac/idc;
        if (rdc < DC_MIN || idc < DC_MIN || pr < PERF_MIN || pi < PERF_MIN) return false;
        Rseg[s] = pr / pi;
    }
    float Rmean = (Rseg[0]+Rseg[1]+Rseg[2])/3.f;
    float Rdev  = fabsf(Rseg[0]-Rmean)+fabsf(Rseg[1]-Rmean)+fabsf(Rseg[2]-Rmean);
    if (Rdev > 0.60f) return false; // antes 0.45f — mais tolerante

    // Calibração linear simples
    float spo2 = 110.0f - 25.0f * R;
    if (spo2 > 100.f) spo2 = 100.f;
    if (spo2 < 70.f)  spo2 = 70.f;

#if DEBUG
    printf("[dbg] IR_DC=%.0f  perf=%.4f  R=%.3f  Rdev=%.3f\n", ir_dc, perf_ir, R, Rdev);
#endif

    *spo2_out = spo2;
    return true;
}

//----------------- MAIN -----------------
int main(void){
    stdio_init_all();
    config_i2c();
    max30102_init();

    sleep_ms(3000);
    uint8_t pid = i2c_read8(REG_PART_ID);
    if (pid != 0x15){
        printf("Atenção: PART_ID=0x%02X (esperado 0x15 p/ MAX30102). Verifique módulo/conexões.\n", pid);
    }else{
        printf("MAX30102 pronto (Part ID: 0x%02X)\n", pid);
    }

    // Estado do detector
    peakdet2_t pd; peakdet2_init(&pd);
    float k_sigma = 0.90f;  // 0.85–1.00 conforme qualidade de sinal
    float bpm_est = 80.f;   // estimativa inicial p/ refratário dinâmico

    // Histórico de picos
    static long sample_counter = 0;
    int peak_idx[MAX_PEAKS]; int n_peaks = 0;

    // SpO2 "hold + EMA"
    static float spo2_smooth = -1.f;
    static int   spo2_hold = 0;

    float ir_ma[SAMPLE_SIZE];

    while (true){
        // Coleta de uma janela (~3 s)
        int n = 0;
        while (n < SAMPLE_SIZE){
            uint32_t red, ir;
            if (max30102_read_sample(&red, &ir)){
                red_buffer[n] = red;
                ir_buffer[n]  = ir;
                n++; sample_counter++;
            }
            sleep_ms(5); // yield leve
        }

        // Suaviza IR para detecção
        moving_average_u32(ir_buffer, ir_ma, SAMPLE_SIZE, MA_WIN);

        // Refratário dinâmico (mais longo para BPM baixo)
        int refract_samples = (int)((bpm_est < 100.f ? 0.35f : 0.25f) * FS_HZ);

        // Detecção de picos
        for (int i=0;i<SAMPLE_SIZE;i++){
            int is_peak = peakdet2_step(&pd, ir_ma[i], k_sigma, refract_samples);
            if (is_peak){
                long abs_idx = sample_counter - SAMPLE_SIZE + i;
                if (n_peaks < MAX_PEAKS) peak_idx[n_peaks++] = (int)abs_idx;
                else {
                    for (int k=1;k<n_peaks;k++) peak_idx[k-1] = peak_idx[k];
                    peak_idx[n_peaks-1] = (int)abs_idx;
                }
            }
        }

        // BPM
        float bpm = bpm_from_last_rr_median(peak_idx, n_peaks, FS_HZ);
        if (bpm > 0.f) bpm_est = bpm;

        // SpO2
        float spo2; bool spo2_ok = calculate_spo2_rr(red_buffer, ir_buffer, SAMPLE_SIZE, &spo2);

        // Hold + EMA para SpO2 (evita "SPO2: --" piscando)
        if (spo2_ok){
            spo2_smooth = (spo2_smooth<0) ? spo2 : (0.2f*spo2 + 0.8f*spo2_smooth);
            spo2_hold = 2; // segura por 2 janelas
            printf("Heart Rate: %.1f BPM\tSpO2: %.1f%%\n", bpm, spo2_smooth);
        }else{
            if (spo2_hold>0 && spo2_smooth>0){
                spo2_hold--;
                printf("Heart Rate: %.1f BPM\tSpO2: %.1f%%\n", bpm, spo2_smooth);
            }else{
                printf("Heart Rate: %.1f BPM\tSpO2: --\n", bpm);
            }
        }
    }
}
