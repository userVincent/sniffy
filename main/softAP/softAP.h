#ifndef SOFTAP_H_
#define SOFTAP_H_

#include <stdint.h>
#include <esp_err.h>

#define SOFTAP_TAG "SOFTAP"
#define MAX_SSID_COUNT 6
#define FREQUENCY_CHANGING_SOFTAP 10000 // 10 seconds	

// Structure to hold the prank information
typedef struct {
    const char **ssids;
    const char **passwords;
    const uint8_t (*mac_addresses)[6];
    uint8_t channel;
    int current_index;
} prank_info_t;

// Create a WiFi SoftAP network interface instance
esp_err_t create_wifi_softap(const char *ssid, const char *password, const uint8_t *mac_address, uint8_t channel);

// Start the prank with multiple APs
esp_err_t start_prank_multiple_aps(const char **ssids, const char **passwords, const uint8_t mac_addresses[][6], uint8_t channel);

// Stop the prank with multiple APs
esp_err_t stop_prank_multiple_aps();

#endif /* SOFTAP_H_ */