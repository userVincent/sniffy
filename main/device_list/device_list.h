#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <stdint.h>
#include <esp_err.h>
#include <stdbool.h>
#include <stdarg.h>

#define DEVICE_LIST_TAG "DEVICE_LIST"

// Linked list node for storing devices
typedef struct device_node_t{
    uint8_t mac_addr[6];    // unique MAC address
    struct device_node_t *next;
} device_node_t;

// wrapper for device_node_t
typedef struct device_list_t{
    device_node_t *head;
    uint32_t size;
    uint8_t channel;
} device_list_t;

// Constructor for device_list_t
device_list_t *device_list_new(uint8_t channel);

// Constructor by combining undifined number device_list_t, whitout diplicate MAC addresses and keeping the channel of the first device_list_t
device_list_t *device_list_new_combine(const device_list_t *device_list1, ...);

// Destructor for device_list_t
esp_err_t device_list_destroy(device_list_t *device_list);

// Add a device using a MAC address to the linked list
esp_err_t device_list_add(const uint8_t *mac_addr, device_list_t *device_list);

// Remove a device using MAC address from the linked list
esp_err_t device_list_remove(const uint8_t *mac_addr, device_list_t *device_list);

// Find a device in the linked list
device_node_t *device_list_find(const uint8_t *mac_addr, const device_list_t *device_list);

// Check if a MAC address is in the linked list
bool device_list_contains(const uint8_t *mac_addr, const device_list_t *device_list);

// Delete all devices from the linked list
esp_err_t device_list_clear(device_list_t *device_list);

// Get all the MAC addresses from the linked list
esp_err_t get_mac_addresses(const device_list_t *device_list, uint8_t *mac_addr, const uint32_t *size);

// Print all devices info in the linked list
esp_err_t device_list_print(const device_list_t *device_list);

#endif // DEVICE_LIST_H