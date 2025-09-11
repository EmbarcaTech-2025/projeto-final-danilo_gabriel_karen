#include "wifi_modulo.h"
#include "pico/cyw43_arch.h"
#include <stdio.h>

bool wifi_connect(const char *ssid, const char *password) {
    if (cyw43_arch_init()) {
        printf("Erro ao inicializar a interface Wi-Fi!\n");
        return false;
    }

    printf("Conectando à rede Wi-Fi: %s\n", ssid);

    cyw43_arch_enable_sta_mode();

    int ret = cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000);
    if (ret != 0) {
        printf("Falha na conexão Wi-Fi\n");
        return false;
    }

    printf("Wi-Fi conectado!\n");

    uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);

    return true;
}

void wifi_disconnect(void) {
    cyw43_arch_deinit();
    printf("Wi-Fi desconectado!\n");
}
