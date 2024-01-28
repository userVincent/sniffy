#include "softAP.h"
#include <esp_wifi.h>
#include <esp_log.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "freertos/event_groups.h"

static prank_info_t *prank_info = NULL;
static TimerHandle_t prank_timer = NULL;

// FUN SONG LYRICS ABOUT OLIVIER
static const char *default_ssids[MAX_SSID_COUNT] = {
    "NEVER GONNA GIVE YOU UP",
    "NEVER GONNA LET YOU DOWN",
    "NEVER GONNA RUN AROUND",	
    "AND DESERT YOU",
    "NEVER GONNA MAKE YOU CRY",
    "NEVER GONNA SAY GOODBYE"
};

static const char *default_passwords[MAX_SSID_COUNT] = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static const uint8_t default_mac_addresses[MAX_SSID_COUNT][6] = {
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01},
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x02},
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x03},
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x04},
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x05},
    {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x06}
};

// Timer callback function
static void prank_ap_switch(TimerHandle_t xTimer) {
    // Get the next SSID, password, and MAC address
    const char *ssid = prank_info->ssids[prank_info->current_index];
    const char *password = prank_info->passwords[prank_info->current_index];
    const uint8_t *mac_address = prank_info->mac_addresses[prank_info->current_index];

    // Create the new SoftAP
    if(create_wifi_softap(ssid, password, mac_address, prank_info->channel) != ESP_OK){
        ESP_LOGE(SOFTAP_TAG, "Failed to create SoftAP");
    }

    // Update the current index
    prank_info->current_index = (prank_info->current_index + 1) % MAX_SSID_COUNT;
}

// Create a WiFi SoftAP network interface instance
esp_err_t create_wifi_softap(const char *ssid, const char *password, const uint8_t *mac_address, uint8_t channel) {
    // Stop current wifi first
    esp_wifi_stop();

    // Destroy existing network interface
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
    if (netif != NULL) {
        esp_netif_destroy(netif);
    }

    // Create network interface for SoftAP
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_WIFI_AP();
    esp_netif_t *wifi_ap = esp_netif_new(&cfg);

    // Attach Wi-Fi driver to network interface
    esp_netif_attach_wifi_station(wifi_ap);
    esp_wifi_set_default_wifi_sta_handlers();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Set MAC address if provided
    if (mac_address != NULL) {
        esp_err_t err = esp_wifi_set_mac(WIFI_IF_AP, mac_address);
        if (err != ESP_OK) {
            ESP_LOGE(SOFTAP_TAG, "Failed to set MAC address");
            return err;
        }
    }

    // Configure SoftAP settings
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "",
            .ssid_len = 0,
            .password = "",
            .ssid_hidden = 0, // Set to 0 to broadcast the SSID
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        }
    };

    password == NULL ? wifi_config.ap.authmode = WIFI_AUTH_OPEN : strlcpy((char *)wifi_config.ap.password, password, sizeof(wifi_config.ap.password));
    strlcpy((char *)wifi_config.ap.ssid, ssid, sizeof(wifi_config.ap.ssid));
    wifi_config.ap.channel = channel;

    // Set the Wi-Fi mode to SoftAP
    esp_err_t err = esp_wifi_set_mode(WIFI_MODE_AP);
    if (err != ESP_OK) {
        return err;
    }

    // Set the SoftAP configuration
    err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (err != ESP_OK) {
        return err;
    }

    // Start the Wi-Fi driver
    return esp_wifi_start();
}


// Start the prank with multiple APs
esp_err_t start_prank_multiple_aps(const char **ssids, const char **passwords, const uint8_t mac_addresses[][6], uint8_t channel) {
    if (ssids == NULL || passwords == NULL) {
        ssids = default_ssids;
        passwords = default_passwords;
    }
    if (mac_addresses == NULL) {
        mac_addresses = default_mac_addresses;
    }

    // Allocate and initialize the prank_info structure
    prank_info = malloc(sizeof(prank_info_t));
    prank_info->ssids = ssids;
    prank_info->passwords = passwords;
    prank_info->mac_addresses = mac_addresses;
    prank_info->channel = channel;
    prank_info->current_index = 0;

    // Create the FreeRTOS timer
    prank_timer = xTimerCreate("prank_timer", pdMS_TO_TICKS(FREQUENCY_CHANGING_SOFTAP), pdTRUE, NULL, prank_ap_switch);
    if (prank_timer == NULL) {
        ESP_LOGE(SOFTAP_TAG, "Failed to create timer");
        free(prank_info);
        return ESP_FAIL;
    }

    // Start the timer
    if (xTimerStart(prank_timer, 0) != pdPASS) {
        ESP_LOGE(SOFTAP_TAG, "Failed to start timer");
        xTimerDelete(prank_timer, 0);
        free(prank_info);
        return ESP_FAIL;
    }
    ESP_LOGI(SOFTAP_TAG, "Prank started");

    return ESP_OK;
}

// Stop the prank with multiple APs
esp_err_t stop_prank_multiple_aps() {
    // Stop timer
    if (xTimerStop(prank_timer, 0) != pdPASS) {
        ESP_LOGE(SOFTAP_TAG, "Failed to stop timer");
        return ESP_FAIL;
    }

    // Delete timer
    if (xTimerDelete(prank_timer, 0) != pdPASS) {
        ESP_LOGE(SOFTAP_TAG, "Failed to delete timer");
        return ESP_FAIL;
    }

    // Free info
    free(prank_info);
    prank_info = NULL;

    // Stop Wi-Fi driver
    esp_wifi_stop();

    ESP_LOGI(SOFTAP_TAG, "Prank stopped");

    return ESP_OK;
}
