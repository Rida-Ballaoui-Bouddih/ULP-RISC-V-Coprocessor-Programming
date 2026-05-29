#ifndef WIFI_H
#define WIFI_H

#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

// WIFI
#define WIFI_SSID                 "Tu Africano Favorito"
#define WIFI_PASS                 "20062006"

/* Prepares the wifi subsystem and connects to the AP */
void wifi_init(void);

/* Returns true if the we are connected to the AP */
bool wifi_is_connected(void);

#endif