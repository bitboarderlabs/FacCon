//-- io_core.h --//
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

typedef enum{
	celcius = 0,
	fahrenheit = 1,
	kelvin = 2
}eTempScale;

// Public API
void IO_Core_Init(ADC_HandleTypeDef* hadc_handle);

// Digital Outputs (num = 0 only)
void IO_SetOutput(uint8_t num, bool value);     // true = on (high)
bool IO_GetOutput(uint8_t num);                 // readback
uint8_t IO_GetOutputLatch(void);

// Digital Inputs (num = 0 only)
uint8_t IO_GetInputBank(void);					// Update the entire bank of inputs
bool IO_GetInput(uint8_t num);                  // true when pressed (high)

// Analog Inputs (num = 0..2)
uint16_t IO_GetAnalogInRaw(uint8_t num);        // Return the raw uint16_t (12-bit) analog input value
float IO_GetAnalogIn(uint8_t num, eTempScale tempScale);	// Return the appropriate number based on the index.  0-3 = temperature, 4 = amperage
float IO_GetTemperature(uint8_t sensorNum, eTempScale tempScale);
float IO_GetAmperage(void);

// Safety function
void IO_AllOutputsOff(void);

#endif /* IO_CORE_H */
