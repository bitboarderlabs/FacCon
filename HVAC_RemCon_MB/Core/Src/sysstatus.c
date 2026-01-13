#include "sysstatus.h"
#include "main.h"
#include "ST7735Canvas.h"

TIM_HandleTypeDef* htimStatus = NULL;
#define TIM_CHAN_STATUSLED 	TIM_CHANNEL_1
#define TIM_CHAN_COMMLED 	TIM_CHANNEL_2
#define TIM_CHAN_ALARMLED 	TIM_CHANNEL_3
#define TIM_CHAN_LCDBL 		TIM_CHANNEL_4

uint8_t onVal_StatusLed = 10;		//Max brightness val
uint8_t onVal_CommLed	= 10;
uint8_t onVal_AlarmLed	= 10;
uint8_t onVal_LcdBl		= 50;

uint8_t curVal_StatusLed 	= 0;	//Currentt val; either 0 (off) or onVal_x (on)
uint8_t curVal_CommLed		= 0;
uint8_t curVal_AlarmLed		= 0;
uint8_t curVal_LcdBl		= 50;

__IO uint32_t*	pCCR_Status = 0;
__IO uint32_t*	pCCR_Comm 	= 0;
__IO uint32_t*	pCCR_Alarm 	= 0;
__IO uint32_t*	pCCR_LcdBl 	= 0;



#define USE_LCD 1
#ifndef PROGMEM
#define PROGMEM
#endif

LCD_HandleTypeDef lcd;
#define DISPLAY_NAME "RemCon"
#define DISPLAY_VER "v1.0"
#include "Dialog10pNarRssi.h"
#include "Dialog24p.h"
const GFXfont *fontBig				= &Dialog_plain_24;
const GFXfont *fontStatusbar		= &Dialog_plain_10NarRssi;

static SystemState_t current_state = SYSTEM_STATE_BOOT;
static uint32_t system_blink_tick = 0;
static uint32_t comms_blink_start = 0;
static uint8_t comms_led_active = 0;

void SysStatus_Init(TIM_HandleTypeDef* hTimerHandle, SPI_HandleTypeDef* hLcdSpiHandle){

	htimStatus = hTimerHandle;

	pCCR_Status = &htimStatus->Instance->CCR1;
	pCCR_Comm = &htimStatus->Instance->CCR2;
	pCCR_Alarm = &htimStatus->Instance->CCR3;
	pCCR_LcdBl = &htimStatus->Instance->CCR4;

	*pCCR_Status = curVal_StatusLed;
	*pCCR_Comm = curVal_CommLed;
	*pCCR_Alarm = curVal_AlarmLed;
	*pCCR_LcdBl = curVal_LcdBl;

	HAL_TIM_Base_Start(htimStatus);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_STATUSLED);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_COMMLED);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_ALARMLED);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_LCDBL);


	ST7735_Init(&lcd, 80, 160, hLcdSpiHandle, LCD_NCS_GPIO_Port, LCD_NCS_Pin, LCD_AO_GPIO_Port, LCD_AO_Pin,
			LCD_NRST_GPIO_Port, LCD_NRST_Pin, 3, INITR_MINI160x80_PLUGIN, true);

	ST7735_SetRotation(&lcd, 3);
	ST7735_FillScreen(&lcd, BLACK);
	ST7735_SetFont(&lcd, fontBig);
	ST7735_SetTextColor(&lcd, WHITE, BLACK);

	uint16_t tmpW = ST7735_GetTextWidth(&lcd, DISPLAY_NAME);
	uint16_t tmpH = ST7735_GetTextHeight(&lcd, DISPLAY_NAME, false);
	ST7735_SetCursor(&lcd, (160-tmpW)/2 , 10 + tmpH);
	ST7735_Print(&lcd, DISPLAY_NAME);
	tmpW = ST7735_GetTextWidth(&lcd, DISPLAY_VER);
	ST7735_SetCursor(&lcd, (160-tmpW)/2 , 10 + tmpH + 10 + tmpH);
	ST7735_Print(&lcd, DISPLAY_VER);

	current_state = SYSTEM_STATE_BOOT;
	system_blink_tick = HAL_GetTick();
	comms_blink_start = 0;
	comms_led_active = 0;

}


void LEDs_SetSystemState(SystemState_t state)
{
    if (current_state != state) {
        current_state = state;
        if (state == SYSTEM_STATE_BOOT) {
            curVal_StatusLed = 0;
        	*pCCR_Status = curVal_StatusLed;
        }
        // When switching to STABLE, next blink will handle the toggle
    }
}

void LED_Comms_Trigger(void)
{
    if (!comms_led_active) {
        curVal_CommLed = onVal_CommLed;
    	*pCCR_Comm = curVal_CommLed;
        comms_blink_start = HAL_GetTick();
        comms_led_active = 1;
    }
}

void LEDs_Process(void)
{
    // Comms LED: 10 ms pulse
    if (comms_led_active) {
        if (HAL_GetTick() - comms_blink_start >= 10) {
            curVal_CommLed = 0;
			*pCCR_Comm = curVal_CommLed;
            comms_led_active = 0;
        }
    }

    // System LED: only active in STABLE state
    if (current_state == SYSTEM_STATE_STABLE) {
        if (HAL_GetTick() - system_blink_tick >= 4000) {  // Toggle every 4 seconds
            curVal_StatusLed = (curVal_StatusLed > 0 ? 0 : onVal_StatusLed);
        	*pCCR_Status = curVal_StatusLed;
            system_blink_tick = HAL_GetTick();
        }
    }
}
