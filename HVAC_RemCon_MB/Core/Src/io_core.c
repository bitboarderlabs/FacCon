#include "io_core.h"
#include "main.h"




// Dig IO Ports & Pins
static GPIO_TypeDef*	DigOutPort[] = {OUT_0_GPIO_Port, OUT_1_GPIO_Port, OUT_2_GPIO_Port, OUT_3_GPIO_Port, OUT_4_GPIO_Port, OUT_5_GPIO_Port, OUT_6_GPIO_Port};
static uint16_t			DigOutPin[DOUT_COUNT] = {OUT_0_Pin, OUT_1_Pin, OUT_2_Pin, OUT_3_Pin, OUT_4_Pin, OUT_5_Pin, OUT_6_Pin};

static GPIO_TypeDef*	DigInPort[DIN_COUNT] = {DIN_1_GPIO_Port, DIN_2_GPIO_Port};
static uint16_t			DigInPin[DIN_COUNT] = {DIN_1_Pin, DIN_2_Pin};

static uint32_t			AnaInVals[AIN_COUNT];

// Latch for digital output readback and safety
static uint8_t output_latch = 0x00;

// Pointer to ADC handle (set in init)
static ADC_HandleTypeDef *hadc_io = NULL;

void IO_Core_Init(ADC_HandleTypeDef* hadc_handle){
	hadc_io = hadc_handle;

    // Ensure output starts off
    IO_AllOutputsOff();

    HAL_ADC_Start_DMA(hadc_io, AnaInVals, AIN_COUNT);
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

bool IO_GetInput(uint8_t num)
{
    if (num >= DIN_COUNT) return false;

    // PC13 = user button, active high when pressed (confirmed by test)
    //GPIO_PinState state = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
    GPIO_PinState state = HAL_GPIO_ReadPin(DigInPort[num], DigInPin[num]);
    return (state == GPIO_PIN_SET);  // true when button pressed
}

uint16_t IO_GetAnalogIn(uint8_t num)
{
    if (num >= AIN_COUNT || hadc_io == NULL) return 0;

    ADC_ChannelConfTypeDef sConfig = {0};
    uint32_t channel;

//    switch (num) {
//        case 0: channel = ADC_CHANNEL_0; break;
//        case 1: channel = ADC_CHANNEL_1; break;
//        case 2: channel = ADC_CHANNEL_2; break;
//        default: return 0;
//    }
//
//    sConfig.Channel      = channel;
//    sConfig.Rank         = 1;  // Fixed: numeric 1 instead of ADC_REGULAR_RANK_1
//    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;  // Fastest
//    sConfig.Offset       = 0;
//
//    if (HAL_ADC_ConfigChannel(hadc_io, &sConfig) != HAL_OK) {
//        return 0;
//    }
//
//    if (HAL_ADC_Start(hadc_io) != HAL_OK) {
//        return 0;
//    }
//
//    // Timeout 10 ms — safe for POC; we'll optimize later if needed
//    if (HAL_ADC_PollForConversion(hadc_io, 10) == HAL_OK) {
//        return (uint16_t)HAL_ADC_GetValue(hadc_io);
//    }
//
//    return 0;

    return AnaInVals[num];
}

void IO_AllOutputsOff(void)
{
    output_latch = 0x00;
    for(uint8_t n=0; n<DOUT_COUNT; n++){
    	HAL_GPIO_WritePin(DigOutPort[n], DigOutPin[n], GPIO_PIN_RESET);
    }
}
