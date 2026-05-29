#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include "esp_event.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"

extern int door_access;

void mqtt_service_init(const char *uri);
int mqtt_service_subscribe(const char *topic);
int  mqtt_service_publish(const char *topic, const char *data, int len);

// take care, it can return true, but the connection could be not ready
// bool mqtt_service_is_connected(void);

#endif