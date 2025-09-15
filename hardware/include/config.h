#ifndef CONFIG_H
#define CONFIG_H

// GPS - Pinagem e configuração
#define GPS_UART_ID uart1
#define GPS_BAUDRATE 9600
#define GPS_TX_PIN 8
#define GPS_RX_PIN 9
#define MAX_NMEA_LEN 100
// Batimentos cardiacos; Oxigenação - Pinagem e configuração
// #define OXI_BAT_I2C_PORT i2c1
// #define OXI_BAT_I2C_SDA  2
// #define OXI_BAT_I2C_SCL  3

// Configuração aceleração - Pinagem e configuração
#define ACELEROMETRO_SDA 0
#define ACELEROMETRO_SCL 1
#define ACELEROMETRO_PORT i2c0
#define ACELEROMETRO_BAUDRATE 400000

// Configuração de WIFI
#define WIFI_SSID "VIVOFIBRA-WIFI6-A2F1"
#define WIFI_PASS "QT3HRPiPAdx5aaL"
// Configuração MQTT
#define MQTT_USUARIO "sistema_idoso"
#define MQTT_IP_BROKER "192.168.15.5"
// Configuração de Software
#define GPS_SEND_INTERVAL_MS 5000
// ID
#define ID_USUARIO 123
// Rotas MQTT
#define GPS_ROUTE "usuario/gps"

#endif // CONFIG_H