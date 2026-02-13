//-- mqtt_client.h --//
#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdbool.h>

// MQTT client states
typedef enum {
    MQTT_STATE_IDLE,
    MQTT_STATE_DNS_LOOKUP,
    MQTT_STATE_CONNECTING,
    MQTT_STATE_SUBSCRIBING,
    MQTT_STATE_CONNECTED,
    MQTT_STATE_RECONNECT_WAIT
} MQTT_State_t;

// Public API
void MQTT_Client_Init(void);
void MQTT_Client_Process(void);
bool MQTT_Client_IsConnected(void);

#endif /* MQTT_CLIENT_H */
