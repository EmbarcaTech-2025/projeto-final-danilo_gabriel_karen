// main.c
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/uart.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Drivers / módulos (presumo que você tenha esses headers)
#include "include/gps_modulo/gps_modulo.h"
#include "include/wifi_modulo/wifi_modulo.h"
#include "include/mqtt_modulo/mqtt_modulo.h"
#include "include/acelerometro_modulo/acelerometro_modulo.h"
#include "include/acelerometro_modulo/detector_queda.h"
#include "include/display_modulo/ssd1306.h"
#include "include/buzzer/buzzer_modulo.h"
#include "include/max30102/max30102.h" // presumido
#include "config.h"

static ssd1306_t g_oled;
static uint32_t g_red[WINDOW_SAMPLES], g_ir[WINDOW_SAMPLES];


static uint8_t detect_max30102_addr(i2c_inst_t *bus) {
    const uint8_t cand[2] = {0x57, 0x5A};
    uint8_t dummy = 0;
    for (int k = 0; k < 2; k++) {
        int r = i2c_write_timeout_us(bus, cand[k], &dummy, 1, true, 1000);
        if (r >= 0) return cand[k];
    }
    return 0;
}


void send_gps_position() {
    printf("Entrou na função blocante\n");
    gps_position_t posicao_atual = get_gps_location(); // função bloqueante do seu driver
    printf("Saiu da função da blocante\n");

    if (!posicao_atual.is_valid) {
        printf("GPS inválido, não enviando.\n");
        return;
    }

    char payload[128];

    int len = snprintf(payload, sizeof(payload),
                       "{\"usuarioId\":%d,\"latitude\":%.6f,\"longitude\":%.6f}",
                       ID_USUARIO, posicao_atual.latitude, posicao_atual.longitude);

    if (len < 0 || len >= sizeof(payload)) {
        printf("Erro ao formatar payload JSON.\n");
        return;
    }

    mqtt_comm_publish(GPS_ROUTE, (const uint8_t*)payload, strlen(payload));
    printf("Mensagem enviada com sucesso\n");
}

void gps_task(void* pvParameters) {
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        send_gps_position();
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(GPS_SEND_INTERVAL_MS));
    }
}


void sensor_task(void* pvParameters) {

    mpu6050_data_t s;
    float ax0 = 0, ay0 = 0, az0 = 0;
    const int Ncal = 100;
    for (int i = 0; i < Ncal; ++i) {
        if (mpu6050_read(&s)) { ax0 += s.ax; ay0 += s.ay; az0 += s.az; }
        sleep_ms(10);
    }
    ax0 /= Ncal; ay0 /= Ncal; az0 /= Ncal;
    fall_detector_set_gravity_ref(ax0, ay0, az0);
    printf("[SENSOR] Calibracao gravidade: ax0=%.3f ay0=%.3f az0=%.3f\n", ax0, ay0, az0);

    
    max30102_t max;
    uint8_t max_addr = detect_max30102_addr(I2C0_PORT);
    max30102_config_t cfg = {
        .i2c = I2C0_PORT,
        .sda_pin = I2C0_SDA,
        .scl_pin = I2C0_SCL,
        .i2c_hz = I2C0_BAUD,
        .i2c_addr = max_addr ? max_addr : MAX30102_I2C_ADDR,
        .led_red_init = 0x24,
        .led_ir_init = 0x24,
        .fifo_avg = 3,
        .sr_code = 3,
        .pw_code = 3,
        .spo2_adc_rng = 1
    };

    bool max_ok = max30102_init(&max, &cfg);
    if (!max_ok) {
        printf("[MAX30102] ERRO de init\n");
    } else {
        printf("[MAX30102] PartID=0x%02X\n", max30102_get_part_id(&max));
    }


    max30102_algo_t algo;
    max30102_algo_default(&algo);

    // Loop de aquisição / processamento
    const uint32_t PERIOD_MS = LOG_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    int idx = 0;
    float bpm = 0, spo2 = 0;
    int bpm_ok = 0, spo2_ok = 0;

    static bool queda_prev = false;
    mpu6050_data_t imu = {0};

    while (1) {
        // Coleta de amostras do MAX30102 até preencher a janela
        int need = (PERIOD_MS * FS_HZ) / 1000 + 2;
        for (int k = 0; k < need && idx < WINDOW_SAMPLES; ++k) {
            uint32_t r, i;
            if (max_ok && max30102_read_sample(&max, &r, &i)) {
                g_red[idx] = r; g_ir[idx] = i; idx++;
            } else break;
        }

        
        if (mpu6050_read(&imu)) {
            
        }

        // Processa janela completa para extrair BPM e SPO2
        if (max_ok && idx >= WINDOW_SAMPLES) {
            float b = 0, s = 0;
            bool ok = max30102_process_window(&algo, g_red, g_ir, &b, &s);
            bpm = b; bpm_ok = (b > 0);
            spo2 = s; spo2_ok = ok && (s > 0);
            max30102_set_leds(&max, algo.led_red, algo.led_ir);
            idx = 0;
        }

        /
        extern bool queda; 
        extern float h;    
        

        
        printf("[LOG] HR=%.0f(%d) SpO2=%.0f(%d) "
               "A[%.2f,%.2f,%.2f] G[%.1f,%.1f,%.1f] queda=%d h=%.2f\n",
               bpm, bpm_ok, spo2, spo2_ok,
               imu.ax, imu.ay, imu.az,
               imu.gx, imu.gy, imu.gz,
               (int)queda, h);

        
        oled_draw_simple(bpm, bpm_ok, spo2, spo2_ok);

        
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(PERIOD_MS));
    }
}

int main(void) {
    stdio_init_all();


    wifi_connect(WIFI_SSID, WIFI_PASS);
    sleep_ms(2000);

    gps_init();
    sleep_ms(1000);

    mqtt_setup(MQTT_USUARIO, MQTT_IP_BROKER, NULL, NULL);
    sleep_ms(500);

    i2c_init(I2C0_PORT, I2C0_BAUD);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);

    buzzer_init();
    mpu6050_init();
    ssd1306_init(&g_oled); 


    BaseType_t r;
    r = xTaskCreate(gps_task, "GPS_Task", 2048/sizeof(StackType_t), NULL, 2, NULL);
    if (r != pdPASS) {
        printf("Falha ao criar GPS_Task\n");
    }

    r = xTaskCreate(sensor_task, "SENSOR_Task", 4096/sizeof(StackType_t), NULL, 3, NULL);
    if (r != pdPASS) {
        printf("Falha ao criar SENSOR_Task\n");
    }
   
    vTaskStartScheduler();

    while (1) {
        printf("Erro: scheduler nao iniciou\n");
        sleep_ms(1000);
    }

    return 0;
}
