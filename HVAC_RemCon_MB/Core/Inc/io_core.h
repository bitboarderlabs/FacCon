#ifndef IO_CORE_H
#define IO_CORE_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// Current I/O configuration
#define DOUT_COUNT  7   // PB14 - red LED (active high)
#define DIN_COUNT   2   // PC13 - user button (active high when pressed)
#define AOUT_COUNT	0
#define AIN_COUNT   5	// ADC1 IN0, IN1, IN2


// Public API
void IO_Core_Init(ADC_HandleTypeDef* hadc_handle);

// Digital Outputs (num = 0 only)
void IO_SetOutput(uint8_t num, bool value);     // true = on (high)
bool IO_GetOutput(uint8_t num);                 // readback

// Digital Inputs (num = 0 only)
bool IO_GetInput(uint8_t num);                  // true when pressed (high)

// Analog Inputs (num = 0..2)
uint16_t IO_GetAnalogIn(uint8_t num);             // 12-bit value

// Safety function
void IO_AllOutputsOff(void);

#endif /* IO_CORE_H */
