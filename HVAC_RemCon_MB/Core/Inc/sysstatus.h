#ifndef SYSSTATUS_H
#define SYSSTATUS_H

#include "main.h"

//#define LED_STATUS_PORT LD1_GPIO_Port
//#define LED_STATUS_PIN  LD1_Pin
//#define LED_COMM_PORT   LD2_GPIO_Port
//#define LED_COMM_PIN    LD2_Pin
//#define LED_ON_STATE    GPIO_PIN_SET
//#define LED_OFF_STATE   GPIO_PIN_RESET

typedef enum {
    SYSTEM_STATE_BOOT = 0,     // LED off
    SYSTEM_STATE_STABLE        // Slow 4-second blink
} SystemState_t;

void SysStatus_Init(TIM_HandleTypeDef* hTimerHandle, SPI_HandleTypeDef* hLcdSpiHandle);

//void LEDs_Init(void);
//void SysStatus_Init_WithLCD(GPIO_TypeDef* statusLedPort, uint16_t statusLedPin, GPIO_PinState statusLedOnState,
//		GPIO_TypeDef* commLedPort, uint16_t commLedPin, GPIO_PinState commLedOnState,
//		GPIO_TypeDef* alarmLedPort, uint16_t alarmLedPin, GPIO_PinState alarmLedOnState,
//		GPIO_TypeDef* lcdBacklightPort, uint16_t lcdBacklightPin, GPIO_PinState lcdBlOnState,
//		SPI_HandleTypeDef* hlcdSpiHandle,
//		GPIO_TypeDef* lcdCsPort, uint16_t lcdCsPin, GPIO_TypeDef* lcdAoPort, uint16_t lcdAoPin,
//		GPIO_TypeDef* lcdResetPort, uint16_t lcdResetPin);
//void SysStatus_Init_NoLCD(GPIO_TypeDef* statusLedPort, uint16_t statusLedPin, GPIO_PinState statusLedOnState,
//		GPIO_TypeDef* commLedPort, uint16_t commLedPin, GPIO_PinState commLedOnState,
//		GPIO_TypeDef* alarmLedPort, uint16_t alarmLedPin, GPIO_PinState alarmLedOnState);
void LEDs_SetSystemState(SystemState_t state);
void LED_Comms_Trigger(void);
void LEDs_Process(void);       // Call every main loop iteration

#endif
