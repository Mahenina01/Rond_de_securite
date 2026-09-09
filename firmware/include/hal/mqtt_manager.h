
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "hal/log_entry.h"

// Prototypes de l'interface mqtt_manager
bool mqtt_connect();
bool mqtt_publish_log(const LogEntry& entry);
bool mqtt_wait_ack(uint32_t timeout_ms);
void mqtt_disconnect();

#endif 