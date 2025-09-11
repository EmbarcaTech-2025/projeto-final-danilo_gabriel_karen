#ifndef WIFI_MODULO_H
#define WIFI_MODULO_H

#include <stdbool.h>

// Inicializa o Wi-Fi com SSID e senha
bool wifi_connect(const char *ssid, const char *password);

// Desconecta do Wi-Fi
void wifi_disconnect(void);

#endif // WIFI_H
