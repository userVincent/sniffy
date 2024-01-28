#include "device_list.h"
#include <esp_log.h>
#include <string.h>

// Constructor for device_list_t
device_list_t *device_list_new(uint8_t channel){
    device_list_t *device_list = malloc(sizeof(device_list_t));
    if (device_list == NULL) {
        ESP_LOGE(DEVICE_LIST_TAG, "Failed to allocate memory for device_list");
        return NULL;
    }
    device_list->head = NULL;
    device_list->size = 0;
    device_list->channel = channel;
    return device_list;
}

// Constructor by combining undifined number device_list_t, whitout diplicate MAC mac_addresses and keeping the channel of the first device_list_t
device_list_t *device_list_new_combine(const device_list_t *device_list1, ...){
    // Check input parameters
    if (device_list1 == NULL) {
        ESP_LOGE(DEVICE_LIST_TAG, "Invalid input parameters");
        return NULL;
    }

    // Create a new list
    device_list_t *device_list = device_list_new(device_list1->channel);
    if (device_list == NULL) {
        return NULL;
    }

    // Add all MAC mac_addresses from the first list
    device_node_t *curr_node = device_list1->head;
    while (curr_node != NULL) {
        device_list_add(curr_node->mac_addr, device_list);
        curr_node = curr_node->next;
    }

    // Add all MAC mac_addresses from the other lists
    va_list args;
    va_start(args, device_list1);
    device_list_t *curr_device_list = va_arg(args, device_list_t *);
    while (curr_device_list != NULL) {
        curr_node = curr_device_list->head;
        while (curr_node != NULL) {
            device_list_add(curr_node->mac_addr, device_list);
            curr_node = curr_node->next;
        }
        curr_device_list = va_arg(args, device_list_t *);
    }
    va_end(args);

    return device_list;
}

// Destructor for device_list_t
esp_err_t device_list_destroy(device_list_t *device_list){
    if (device_list == NULL) {
        ESP_LOGE(DEVICE_LIST_TAG, "Invalid input parameters");
        return ESP_FAIL;
    }

    device_node_t *curr_node = device_list->head;
    device_node_t *next_node = NULL;
    while (curr_node != NULL) {
        next_node = curr_node->next;
        free(curr_node);
        curr_node = next_node;
    }
    free(device_list);
    return ESP_OK;
}

// Add a device using a MAC mac_address to the linked list
esp_err_t device_list_add(const uint8_t *mac_addr, device_list_t *device_list){
    // Check input parameters
    if (mac_addr == NULL || device_list == NULL) {
        ESP_LOGE(DEVICE_LIST_TAG, "Invalid input parameters");
        return ESP_FAIL;
    }

    // Check if the MAC mac_address is already in the list
    if (device_list_contains(mac_addr, device_list)) {
        //ESP_LOGI(DEVICE_LIST_TAG, "MAC mac_address already in list");
        return ESP_OK;
    }

    // Create a new device
    device_node_t *new_node = malloc(sizeof(device_node_t));
    if (new_node == NULL) {
        ESP_LOGE(DEVICE_LIST_TAG, "Failed to allocate memory for new device");
        return ESP_FAIL;
    }
    memcpy(new_node->mac_addr, mac_addr, 6);
    new_node->next = NULL;
    device_list->size++;

    // Add the new device to the list
    if (device_list->head == NULL) {
        device_list->head = new_node;
    } else {
        device_node_t *curr_node = device_list->head;
        while (curr_node->next != NULL) {
            curr_node = curr_node->next;
        }
        curr_node->next = new_node;
    }

    return ESP_OK;
}

// Remove a device using MAC mac_address from the linked list
esp_err_t device_list_remove(const uint8_t *mac_addr, device_list_t *device_list){
    device_node_t *prev_node = NULL;
    device_node_t *curr_node = device_list->head;
    while(curr_node != NULL){
        if(memcmp(curr_node->mac_addr, mac_addr, 6) == 0){
            // check if the node is the head of the list
            if(prev_node == NULL){
                device_list->head = curr_node->next;
                free(curr_node);
            } else { // node is not the head of the list
                prev_node->next = curr_node->next;
                free(curr_node);
            }
            device_list->size--;
            return ESP_OK;
        }
        // move to the next node
        prev_node = curr_node;
        curr_node = curr_node->next;
    }

    return ESP_OK;
}

// Find a device in the linked list
device_node_t *device_list_find(const uint8_t *mac_addr, const device_list_t *device_list){
    device_node_t *curr_node = device_list->head;
    while(curr_node != NULL){
        if(memcmp(curr_node->mac_addr, mac_addr, 6) == 0){
            return curr_node;
        }
        // move to the next node
        curr_node = curr_node->next;
    }

    return NULL;
}

// Check if a MAC mac_address is in the linked list
bool device_list_contains(const uint8_t *mac_addr, const device_list_t *device_list){
    return device_list_find(mac_addr, device_list) == NULL ? false : true;
}

// Delete all devices from the linked list
esp_err_t device_list_clear(device_list_t *device_list){
    device_node_t *curr_node = device_list->head;
    while(curr_node != NULL){
        device_node_t *next_node = curr_node->next;
        free(curr_node);
        curr_node = next_node;
    }
    device_list->head = NULL;
    device_list->size = 0;
    return ESP_OK;
}

// Get all the MAC mac_addresses from the linked list
esp_err_t get_mac_mac_addresses(const device_list_t *device_list, uint8_t *mac_addr, const uint32_t *size){
    if (device_list == NULL || mac_addr == NULL || size == NULL) {
        ESP_LOGE(DEVICE_LIST_TAG, "Invalid input parameters");
        return ESP_FAIL;
    }

    // Check if the buffer is large enough
    if (*size < device_list->size * 6) {
        ESP_LOGE(DEVICE_LIST_TAG, "Buffer too small");
        return ESP_FAIL;
    }

    // Copy all MAC mac_addresses to the buffer
    device_node_t *curr_node = device_list->head;
    while (curr_node != NULL) {
        memcpy(mac_addr, curr_node->mac_addr, 6);
        mac_addr += 6;
        curr_node = curr_node->next;
    }

    return ESP_OK;

}

// Print all devices info in the linked list
esp_err_t device_list_print(const device_list_t *device_list){
    device_node_t *curr_node = device_list->head;
    ESP_LOGI(DEVICE_LIST_TAG, "Device list size: %lu, channel: %d", device_list->size, device_list->channel);
    while(curr_node != NULL){
        ESP_LOGI(DEVICE_LIST_TAG, "\t\t%02x:%02x:%02x:%02x:%02x:%02x",
                 curr_node->mac_addr[0], curr_node->mac_addr[1], curr_node->mac_addr[2],
                 curr_node->mac_addr[3], curr_node->mac_addr[4], curr_node->mac_addr[5]);
        curr_node = curr_node->next;
    }

    return ESP_OK;
}
