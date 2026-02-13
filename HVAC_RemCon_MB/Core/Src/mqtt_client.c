//-- mqtt_client.c --//
#include "mqtt_client.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/netif.h"
#include "io_core.h"
#include <string.h>
#include <stdio.h>

// Broker configuration
#define MQTT_BROKER_HOSTNAME    "iot.st-el.com"
#define MQTT_BROKER_PORT        1883

// Topic configuration
#define TOPIC_PREFIX            "/faccon/dev1/"
#define INPUT_TOPIC_PREFIX      TOPIC_PREFIX "in"
#define OUTPUT_TOPIC_PREFIX     TOPIC_PREFIX "out"

// Timing
#define INPUT_POLL_INTERVAL_MS  50
#define RECONNECT_DELAY_MS      5000

// Number of output topics to subscribe to
#define OUTPUT_TOPIC_COUNT      DOUT_COUNT

// External reference to network interface
extern struct netif gnetif;

// State machine
static MQTT_State_t mqtt_state = MQTT_STATE_IDLE;
static mqtt_client_t *mqtt_client = NULL;
static ip_addr_t broker_ip;

// Timing
static uint32_t last_input_poll = 0;
static uint32_t reconnect_start_time = 0;

// Input state tracking
static uint8_t last_input_bank = 0xFF;  // Initialize to invalid state to force initial publish

// Subscription tracking
static uint8_t subscribe_index = 0;

// Incoming message buffers
static char incoming_topic[64];
static uint8_t incoming_payload[8];
static uint16_t incoming_payload_len = 0;

// Client ID buffer
static char client_id[32];

// Forward declarations
static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len);
static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);
static void mqtt_subscribe_cb(void *arg, err_t err);
static void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *arg);
static void start_connection(void);
static void subscribe_next_topic(void);
static void poll_and_publish_inputs(void);
static void handle_output_message(const char *topic, const uint8_t *payload, uint16_t len);
static void enter_reconnect_state(void);
static void cleanup_client(void);
static bool is_network_ready(void);

void MQTT_Client_Init(void)
{
    mqtt_state = MQTT_STATE_IDLE;
    mqtt_client = NULL;
    last_input_bank = 0xFF;
    subscribe_index = 0;

    // Generate client ID based on IP address (will be set when network is ready)
    snprintf(client_id, sizeof(client_id), "faccon_dev1");
}

void MQTT_Client_Process(void)
{
    uint32_t now = HAL_GetTick();

    switch (mqtt_state) {
        case MQTT_STATE_IDLE:
            // Wait for network to be ready
            if (is_network_ready()) {
                // Update client ID with IP
                uint32_t ip = gnetif.ip_addr.addr;
                snprintf(client_id, sizeof(client_id), "faccon_%02x%02x%02x%02x",
                         (uint8_t)(ip), (uint8_t)(ip >> 8),
                         (uint8_t)(ip >> 16), (uint8_t)(ip >> 24));

                // Start DNS lookup
                mqtt_state = MQTT_STATE_DNS_LOOKUP;
                err_t err = dns_gethostbyname(MQTT_BROKER_HOSTNAME, &broker_ip, dns_found_cb, NULL);
                if (err == ERR_OK) {
                    // IP was cached, proceed directly to connection
                    start_connection();
                } else if (err != ERR_INPROGRESS) {
                    // DNS lookup failed immediately
                    enter_reconnect_state();
                }
                // If ERR_INPROGRESS, callback will be called later
            }
            break;

        case MQTT_STATE_DNS_LOOKUP:
            // Waiting for DNS callback
            break;

        case MQTT_STATE_CONNECTING:
            // Waiting for connection callback
            break;

        case MQTT_STATE_SUBSCRIBING:
            // Waiting for subscribe callbacks
            break;

        case MQTT_STATE_CONNECTED:
            // Poll inputs and publish changes
            if (now - last_input_poll >= INPUT_POLL_INTERVAL_MS) {
                poll_and_publish_inputs();
                last_input_poll = now;
            }

            // Check if still connected
            if (mqtt_client && !mqtt_client_is_connected(mqtt_client)) {
                enter_reconnect_state();
            }
            break;

        case MQTT_STATE_RECONNECT_WAIT:
            // Wait for reconnect delay
            if (now - reconnect_start_time >= RECONNECT_DELAY_MS) {
                cleanup_client();
                mqtt_state = MQTT_STATE_IDLE;
            }
            break;
    }
}

bool MQTT_Client_IsConnected(void)
{
    return (mqtt_state == MQTT_STATE_CONNECTED) &&
           mqtt_client &&
           mqtt_client_is_connected(mqtt_client);
}

static bool is_network_ready(void)
{
    return netif_is_link_up(&gnetif) &&
           (gnetif.ip_addr.addr != 0) &&
           (gnetif.ip_addr.addr != IPADDR_ANY);
}

static void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *arg)
{
    (void)name;
    (void)arg;

    if (mqtt_state != MQTT_STATE_DNS_LOOKUP) {
        return;
    }

    if (ipaddr && !ip_addr_isany(ipaddr)) {
        ip_addr_copy(broker_ip, *ipaddr);
        start_connection();
    } else {
        // DNS lookup failed
        enter_reconnect_state();
    }
}

static void start_connection(void)
{
    // Allocate new client if needed
    if (!mqtt_client) {
        mqtt_client = mqtt_client_new();
        if (!mqtt_client) {
            enter_reconnect_state();
            return;
        }
    }

    // Prepare connection info
    struct mqtt_connect_client_info_t ci;
    memset(&ci, 0, sizeof(ci));
    ci.client_id = client_id;
    ci.client_user = NULL;
    ci.client_pass = NULL;
    ci.keep_alive = 60;  // 60 seconds keepalive
    ci.will_topic = NULL;
    ci.will_msg = NULL;
    ci.will_qos = 0;
    ci.will_retain = 0;

    mqtt_state = MQTT_STATE_CONNECTING;

    err_t err = mqtt_client_connect(mqtt_client, &broker_ip, MQTT_BROKER_PORT,
                                    mqtt_connection_cb, NULL, &ci);
    if (err != ERR_OK) {
        enter_reconnect_state();
    }
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
    (void)arg;

    if (status == MQTT_CONNECT_ACCEPTED) {
        // Connected successfully - set up incoming message callbacks
        // NOTE: Must be done AFTER connect, as mqtt_client_connect() memsets the client
        mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, NULL);

        // Start subscribing
        mqtt_state = MQTT_STATE_SUBSCRIBING;
        subscribe_index = 0;
        subscribe_next_topic();
    } else {
        // Connection failed or disconnected
        enter_reconnect_state();
    }
}

static void subscribe_next_topic(void)
{
    if (subscribe_index >= OUTPUT_TOPIC_COUNT) {
        // All subscriptions done
        mqtt_state = MQTT_STATE_CONNECTED;
        last_input_poll = HAL_GetTick();

        // Force initial publish of all inputs
        last_input_bank = 0xFF;
        return;
    }

    // Build topic string
    char topic[32];
    snprintf(topic, sizeof(topic), "%s%d", OUTPUT_TOPIC_PREFIX, subscribe_index);

    err_t err = mqtt_subscribe(mqtt_client, topic, 0, mqtt_subscribe_cb, NULL);
    if (err != ERR_OK) {
        enter_reconnect_state();
    }
}

static void mqtt_subscribe_cb(void *arg, err_t err)
{
    (void)arg;

    if (mqtt_state != MQTT_STATE_SUBSCRIBING) {
        return;
    }

    if (err != ERR_OK) {
        enter_reconnect_state();
        return;
    }

    // Move to next subscription
    subscribe_index++;
    subscribe_next_topic();
}

static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
{
    (void)arg;

    // Store topic for use in data callback
    strncpy(incoming_topic, topic, sizeof(incoming_topic) - 1);
    incoming_topic[sizeof(incoming_topic) - 1] = '\0';
    incoming_payload_len = 0;

    // If no payload, we won't get a data callback
    if (tot_len == 0) {
        handle_output_message(incoming_topic, NULL, 0);
    }
}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    (void)arg;

    // Accumulate data (handle fragmentation)
    uint16_t copy_len = len;
    if (incoming_payload_len + copy_len > sizeof(incoming_payload) - 1) {
        copy_len = sizeof(incoming_payload) - 1 - incoming_payload_len;
    }

    if (copy_len > 0) {
        memcpy(&incoming_payload[incoming_payload_len], data, copy_len);
        incoming_payload_len += copy_len;
    }

    // If this is the last fragment, process the message
    if (flags & MQTT_DATA_FLAG_LAST) {
        incoming_payload[incoming_payload_len] = '\0';
        handle_output_message(incoming_topic, incoming_payload, incoming_payload_len);
    }
}

static void handle_output_message(const char *topic, const uint8_t *payload, uint16_t len)
{
    // Parse topic to extract output number
    // Expected format: /faccon/dev1/outN
    const char *prefix = OUTPUT_TOPIC_PREFIX;
    size_t prefix_len = strlen(prefix);

    if (strncmp(topic, prefix, prefix_len) != 0) {
        return;  // Not an output topic
    }

    // Get output number from topic
    const char *num_str = topic + prefix_len;
    int output_num = num_str[0] - '0';

    if (output_num < 0 || output_num >= DOUT_COUNT) {
        return;  // Invalid output number
    }

    // Parse payload: '1' = true, anything else = false
    bool value = false;
    if (payload && len > 0) {
        value = (payload[0] == '1');
    }

    // Set the output
    IO_SetOutput((uint8_t)output_num, value);
}

static void poll_and_publish_inputs(void)
{
    if (!mqtt_client || !mqtt_client_is_connected(mqtt_client)) {
        return;
    }

    uint8_t current_input_bank = IO_GetInputBank();

    // Check for changes using XOR
    uint8_t changed = current_input_bank ^ last_input_bank;

    if (changed == 0) {
        return;  // No changes
    }

    // Publish each changed input
    for (uint8_t i = 0; i < DIN_COUNT; i++) {
        if (changed & (1 << i)) {
            char topic[32];
            char payload[2];

            snprintf(topic, sizeof(topic), "%s%d", INPUT_TOPIC_PREFIX, i);
            payload[0] = (current_input_bank & (1 << i)) ? '1' : '0';
            payload[1] = '\0';

            // Publish with QoS 0, no retain
            mqtt_publish(mqtt_client, topic, payload, 1, 0, 0, NULL, NULL);
        }
    }

    last_input_bank = current_input_bank;
}

static void enter_reconnect_state(void)
{
    mqtt_state = MQTT_STATE_RECONNECT_WAIT;
    reconnect_start_time = HAL_GetTick();
}

static void cleanup_client(void)
{
    if (mqtt_client) {
        if (mqtt_client_is_connected(mqtt_client)) {
            mqtt_disconnect(mqtt_client);
        }
        mqtt_client_free(mqtt_client);
        mqtt_client = NULL;
    }
    subscribe_index = 0;
}
