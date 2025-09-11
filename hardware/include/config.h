#ifndef CONFIG_H
#define CONFIG_H

// GPS - Pinagem e configuração
#define GPS_UART_ID uart1
#define GPS_BAUDRATE 9600
#define GPS_TX_PIN 0
#define GPS_RX_PIN 1
#define MAX_NMEA_LEN 100

// Configuração de WIFI
#define WIFI_SSID "VIVOFIBRA-WIFI6-A2F1"
#define WIFI_PASS "QT3HRPiPAdx5aaL"
// Configuração MQTT
#define MQTT_USUARIO "sistema_idoso"
#define MQTT_IP_BROKER "192.168.15.5"


#endif // CONFIG_H