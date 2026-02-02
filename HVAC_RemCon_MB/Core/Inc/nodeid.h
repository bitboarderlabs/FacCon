//-- nodeid.h --//
#ifndef NODEID_H_
#define NODEID_H_
#include <stdint.h>

extern uint8_t nodeid;           // From your DIP switches
extern char mqtt_root_topic[64];  // e.g. "/hvac/remcon/"

uint8_t NodeId_Get(void);
#endif
