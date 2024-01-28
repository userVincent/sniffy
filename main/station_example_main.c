
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "deauth/deauth.h"
#include "softap/softap.h"

void init(void){
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Initialize the event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize the Wi-Fi library
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Start the Wi-Fi driver
    ESP_ERROR_CHECK(esp_wifi_start()); 
}


void app_main(void)
{
    init();

    // sniff all channels for MAC addresses
    /*
    start_sniffer(1);
    display_devices_info(1);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    */

    // sniff all channels for APs
    /*
    start_sniffer_AP();
    display_APs_info();
    */

    // start DoS attack on seperate task
    /*
    uint8_t AP_mac[6] = {0x34, 0x2c, 0xc4, 0xad, 0xb2, 0xd5};
    uint8_t target_mac[6] = {0x28, 0x7F, 0xCF, 0xBA, 0x0F, 0x59};
    start_dos_attack(AP_mac, target_mac);
    vTaskDelay(60000 / portTICK_PERIOD_MS);
    stop_dos_attack();
    */

   // prank multiple APs
    start_prank_multiple_aps(NULL, NULL, NULL, 1);
    vTaskDelay(600000 / portTICK_PERIOD_MS);
    stop_prank_multiple_aps();
}