//-- sysstatus.h --//
#ifndef SYSSTATUS_H
#define SYSSTATUS_H

#include "main.h"
#include "stdint.h"

typedef enum {
    SYSTEM_STATE_UNKNOWN = 0,	//Initial state
	SYSTEM_STATE_BOOT,     		// All LEDs on (test pattern)
    SYSTEM_STATE_STABLE        	// Slow 4-second blink
} SystemState_t;

void SysStatus_Init(TIM_HandleTypeDef* hTimerHandle, SPI_HandleTypeDef* hLcdSpiHandle);

void SysStatus_SetSystemState(SystemState_t state);
void SysStatus_Comms_Trigger(void);
void SysStatus_Process(void);       // Call every main loop iteration

void SysStatus_UpdateIP(uint32_t ip_addr);
void SysStatus_UpdateNodeId(uint8_t id);

void SysStatus_UpdateInputsDisplay(uint8_t newVals);
void SysStatus_UpdateOutputsDisplay(uint8_t newVals);
void SysStatus_UpdateTempDisplay(uint8_t idx, float newTemp);

#endif
