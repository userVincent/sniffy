#ifndef DEAUTH_H
#define DEAUTH_H

#include <stdint.h>
#include <esp_err.h>

#define DEAUTH_TAG "DEAUTH"

typedef struct {
    uint8_t AP_mac[6];
    uint8_t target_mac[6];
} deauth_info_t;

// start sniffer, channel = 0 means all channels
esp_err_t start_sniffer(u_int8_t channel);

// display all devices in a channel, if channel = 0, display all channels
esp_err_t display_devices_info(u_int8_t channel);

// sniff all APs
esp_err_t start_sniffer_AP();

// display all APs
esp_err_t display_APs_info();

// start DoS attack
esp_err_t start_dos_attack(uint8_t *AP_mac, uint8_t *target_mac);

// stop DoS attack
esp_err_t stop_dos_attack();

#endif // DEAUTH_H