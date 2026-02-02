//-- io_core.c --//
#include "io_core.h"
#include "main.h"
#include "math.h"




// Dig IO Ports & Pins
static GPIO_TypeDef*	DigOutPort[] = {OUT_0_GPIO_Port, OUT_1_GPIO_Port, OUT_2_GPIO_Port, OUT_3_GPIO_Port, OUT_4_GPIO_Port, OUT_5_GPIO_Port, OUT_6_GPIO_Port};
static uint16_t			DigOutPin[DOUT_COUNT] = {OUT_0_Pin, OUT_1_Pin, OUT_2_Pin, OUT_3_Pin, OUT_4_Pin, OUT_5_Pin, OUT_6_Pin};

static GPIO_TypeDef*	DigInPort[DIN_COUNT] = {DIN_1_GPIO_Port, DIN_2_GPIO_Port};
static uint16_t			DigInPin[DIN_COUNT] = {DIN_1_Pin, DIN_2_Pin};

static uint32_t			AnaInVals[AIN_COUNT]; 	//Order: [0]= uC internal temp, [1]= onboard thermistor, [2] = RT1,
													// [3] = RT2, [4] = current transformer

float BVals[3] = {3435.0f, 3300.0f, 3650.0f};  //onboard NTC-MF52-103 = 10k/B=3435.  Honeywell wallmount = 10k/B=3300
#define TS_CAL1_ADDR  ((uint16_t*)0x1FFF7A2C)
#define TS_CAL2_ADDR  ((uint16_t*)0x1FFF7A2E)
uint16_t ts_cal1 = 0;
uint16_t ts_cal2 = 0;

eTempScale curTempScale = fahrenheit;


// Latch for digital output readback and safety
static uint8_t output_latch = 0x00;

// Pointer to ADC handle (set in init)
static ADC_HandleTypeDef *hadc_io = NULL;

float thermistor_to_celsius(uint16_t adc_raw, float Bval);
float get_cpu_temp_calibrated(uint16_t adc_raw);

void IO_Core_Init(ADC_HandleTypeDef* hadc_handle){
	hadc_io = hadc_handle;

    // Ensure output starts off
    IO_AllOutputsOff();

    HAL_ADC_Start_DMA(hadc_io, AnaInVals, AIN_COUNT);
    ts_cal1 = *TS_CAL1_ADDR;
    ts_cal2 = *TS_CAL2_ADDR;
}

void IO_SetOutput(uint8_t num, bool value)
{
    if (num >= DOUT_COUNT) return;

    // Update software latch
    if (value) {
        output_latch |= (1U << num);
    } else {
        output_latch &= ~(1U << num);
    }

    // Hardware: PB14 = red LED (LD3), active high
    //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DigOutPort[num], DigOutPin[num], value ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

bool IO_GetOutput(uint8_t num)
{
    if (num >= DOUT_COUNT) return false;
    return (output_latch & (1U << num)) != 0;
}

uint8_t IO_GetOutputLatch(void){
	return output_latch;
}

bool IO_GetInput(uint8_t num)
{
    if (num >= DIN_COUNT) return false;

    // PC13 = user button, active high when pressed (confirmed by test)
    //GPIO_PinState state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    GPIO_PinState state = HAL_GPIO_ReadPin(DigInPort[num], DigInPin[num]);
    return (state == GPIO_PIN_SET);  // true when button pressed
}

uint8_t IO_GetInputBank(void){
	uint8_t inVals = 0;
	for(uint8_t n=0; n<DIN_COUNT; n++){
		inVals |= (HAL_GPIO_ReadPin(DigInPort[n], DigInPin[n]) == GPIO_PIN_SET ? (1 << n) : 0);
	}
	return inVals;
}


uint16_t IO_GetAnalogInRaw(uint8_t num)
{
    if (num >= AIN_COUNT || hadc_io == NULL) return 0;

    return AnaInVals[num];
}

float IO_GetAnalogIn(uint8_t num, eTempScale tempScale){
	//This will return the appropriate value depending on what the index number is.
	// 0-3 returns temperature value.
	// 4 returns current value.
	// >4 returns 0
	if(num > 4) return 0;
	if(num == 4){
		return IO_GetAmperage();
	}else{
		return IO_GetTemperature(num, tempScale);
	}
}

float thermistor_to_celsius(uint16_t adc_raw, float Bval)
{
    if (adc_raw == 0)       return NAN;          // avoid div/0
    if (adc_raw >= 4095)    return -99.0f;       // saturated low (very cold)
    if (Bval <= 0.0f)       return NAN;

    float adc = (float)adc_raw;

    // Correct resistance for pull-down NTC (common STM32 setup)
    // Rt = R_fixed * (adc / (4095 - adc))
    float Rt = 10000.0f * (adc / (4095.0f - adc));

    // Beta formula (Steinhart-Hart approximation using Beta)
    const float T0 = 298.15f;      // 25°C in Kelvin
    const float R0 = 10000.0f;     // 10k @ 25°C

    float inv_T = (1.0f / T0) + (1.0f / Bval) * logf(Rt / R0);

    float temp_kelvin = 1.0f / inv_T;
    float temp_celsius = temp_kelvin - 273.15f;

    return temp_celsius;
}

float get_cpu_temp_calibrated(uint16_t adc_raw){
    if (ts_cal2 <= ts_cal1) return NAN; // invalid calibration

    float temp = 30.0f +
                 (110.0f - 30.0f) *
                 ((float)adc_raw - ts_cal1) /
                 (ts_cal2 - ts_cal1);

    return temp;
}

float IO_GetTemperature(uint8_t sensorNum, eTempScale tempScale){
	if(sensorNum > 3) return 0;
	// 0 = uC internal temp
	// 1 = onboard thermistor temp
	// 2 = RT1 external thermistor
	// 3 = RT2 external thermistor

	float tempC = 0.0f;
	if(sensorNum == 0){  // cpu internal temp
		tempC = get_cpu_temp_calibrated((uint16_t)AnaInVals[0]);
	}else{	// thermistor temp
		tempC = thermistor_to_celsius((uint16_t)AnaInVals[sensorNum], BVals[sensorNum - 1]);
	}

	if(tempScale == celcius){
		return tempC;
	}else{
		return tempC * 9 / 5 + 32;
	}
}

float IO_GetAmperage(void){
	return 2.345;
}

void IO_AllOutputsOff(void)
{
    output_latch = 0x00;
    for(uint8_t n=0; n<DOUT_COUNT; n++){
    	HAL_GPIO_WritePin(DigOutPort[n], DigOutPin[n], GPIO_PIN_RESET);
    }
}
