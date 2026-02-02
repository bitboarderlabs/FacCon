//-- sysstatus.c --//
#include "sysstatus.h"
#include "main.h"
#include "ST7735Canvas.h"
#include "stdio.h"


TIM_HandleTypeDef* htimStatus = NULL;
#define TIM_CHAN_STATUSLED 	TIM_CHANNEL_1
#define TIM_CHAN_COMMLED 	TIM_CHANNEL_2
#define TIM_CHAN_ALARMLED 	TIM_CHANNEL_3
#define TIM_CHAN_LCDBL 		TIM_CHANNEL_4

uint8_t onVal_StatusLed = 10;		//Max brightness val
uint8_t onVal_CommLed	= 30;
uint8_t onVal_AlarmLed	= 10;
uint8_t onVal_LcdBl		= 50;

uint8_t curVal_StatusLed 	= 0;	//Current val; either 0 (off) or onVal_x (on)
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

//LCD_CanvasHandleTypeDef canvScreen;
//LCD_CanvasHandleTypeDef canvIpAddr;

uint16_t bgColor = BLACK;
uint16_t fgColor = WHITE;







static SystemState_t current_state = SYSTEM_STATE_UNKNOWN;
static uint32_t system_blink_tick = 0;
static uint32_t comms_blink_start = 0;
static uint8_t comms_led_active = 1;

static uint8_t alarm_led_active = 1;

//void clearScreen();

void SysStatus_Init(TIM_HandleTypeDef* hTimerHandle, SPI_HandleTypeDef* hLcdSpiHandle){

	htimStatus = hTimerHandle;

	pCCR_Status = &htimStatus->Instance->CCR1;
	pCCR_Comm = &htimStatus->Instance->CCR2;
	pCCR_Alarm = &htimStatus->Instance->CCR3;
	pCCR_LcdBl = &htimStatus->Instance->CCR4;

	//Turn on all led's at startup
	*pCCR_Status = onVal_StatusLed;
	*pCCR_Comm = onVal_CommLed;
	*pCCR_Alarm = onVal_AlarmLed;
	*pCCR_LcdBl = onVal_LcdBl;

	HAL_TIM_Base_Start(htimStatus);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_STATUSLED);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_COMMLED);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_ALARMLED);
	HAL_TIM_PWM_Start(htimStatus, TIM_CHAN_LCDBL);


	ST7735_Init(&lcd, 160, 80, hLcdSpiHandle, LCD_NCS_GPIO_Port, LCD_NCS_Pin, LCD_AO_GPIO_Port, LCD_AO_Pin,
			LCD_NRST_GPIO_Port, LCD_NRST_Pin, 1, INITR_MINI160x80_PLUGIN, true);

	ST7735_SetRotation(&lcd, 1);
	//ST7735_InitCanvas(&lcd, &canvScreen, 160, 80);
	//ST7735_InitCanvas(&lcd, &canvIpAddr, 110, 12);

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

	current_state = SYSTEM_STATE_UNKNOWN;
	system_blink_tick = HAL_GetTick();
	comms_blink_start = 0;
	comms_led_active = 0;

}


void SysStatus_SetSystemState(SystemState_t state)
{
    if (current_state != state) {
        current_state = state;
        switch(state){
        case SYSTEM_STATE_UNKNOWN:
        	break;
        case SYSTEM_STATE_BOOT:
        	//Turn off all status led's
        	curVal_StatusLed = 0;
        	*pCCR_Status = curVal_StatusLed;
        	curVal_CommLed = 0;
			*pCCR_Comm = curVal_CommLed;
			curVal_AlarmLed = 0;
			*pCCR_Alarm = curVal_AlarmLed;

			ST7735_FillRect(&lcd, 0, 0, lcd.width, lcd.height, bgColor);
			SysStatus_UpdateIP(0);
        	break;
        case SYSTEM_STATE_STABLE:
        	// When switching to STABLE, next blink will handle the toggle
        	break;
        }
    }
}

void SysStatus_Comms_Trigger(void)
{
    if (!comms_led_active) {
        curVal_CommLed = onVal_CommLed;
    	*pCCR_Comm = curVal_CommLed;
        comms_blink_start = HAL_GetTick();
        comms_led_active = 1;
    }
}

void SysStatus_Process(void)
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



void SysStatus_UpdateIP(uint32_t ip_addr){
	char ip_str[21];

	// Format as dotted decimal (big-endian network order)
	sprintf(ip_str, "IP: %d.%d.%d.%d",
			(uint8_t)(ip_addr & 0xFF),
			(uint8_t)((ip_addr >> 8) & 0xFF),
			(uint8_t)((ip_addr >> 16) & 0xFF),
			(uint8_t)((ip_addr >> 24) & 0xFF));

	ST7735_FillRect(&lcd, 0, 0, 110, 12, bgColor);
	ST7735_SetFont(&lcd, fontStatusbar);
	ST7735_SetTextColor(&lcd, fgColor, bgColor);
	ST7735_SetCursor(&lcd, 2 , 7);
	ST7735_Print(&lcd, ip_str);

//	ST7735_FillRectCanvas(&canvIpAddr, 0, 0, canvIpAddr.width, canvIpAddr.height, bgColor);
//	ST7735_SetFontCanvas(&canvIpAddr, fontStatusbar);
//	ST7735_SetTextColorCanvas(&canvIpAddr, fgColor, bgColor);
//	ST7735_SetCursorCanvas(&canvIpAddr, 2 , 10);
//	ST7735_PrintTextCanvas(&canvIpAddr, ip_str);
//	//ST7735_WriteCanvasDMA(&canvScreen);
//	//ST7735_SetAddressWindow(&lcd, 0, 0, canvIpAddr.width-1, canvIpAddr.height-1);
//	//ST7735_WriteCanvas(&canvIpAddr);
//	ST7735_DrawImage(&lcd, 20, 20, canvIpAddr.width, canvIpAddr.height, canvIpAddr.buffer);
}

void SysStatus_UpdateNodeId(uint8_t id){
	char id_str[21] = {0};
	sprintf(id_str, "ID: %d", id);
	ST7735_FillRect(&lcd, 110, 0, 50, 12, bgColor);
	ST7735_SetFont(&lcd, fontStatusbar);
	ST7735_SetTextColor(&lcd, fgColor, bgColor);
	ST7735_SetCursor(&lcd, 112 , 7);
	ST7735_Print(&lcd, id_str);
}

//void clearScreen(void){
//	ST7735_FillScreen(&lcd, BLACK);
//}

void SysStatus_UpdateInputsDisplay(uint8_t newVals){
	//draw both inputs near the top right

	uint8_t tmpW = 30;
	uint8_t tmpSpace = 4;
	uint8_t tmpTop = 18;
	uint8_t tmpH = 14;
	uint8_t tmpLeft = 159 - ((2 * tmpW) + tmpSpace);

	for(uint8_t n=0; n<2; n++){
		char lbl[5] = {0};
		uint8_t curVal = newVals & (1 << n);
		//Draw the box and set text color:
		if(curVal == 0){ //curVal = 0
			ST7735_FillRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, bgColor);
			ST7735_DrawRoundRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, 3, fgColor);
			ST7735_SetTextColor(&lcd, fgColor, bgColor);
		}else{	//curVal = 1
			//ST7735_FillRoundRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, 3, fgColor);
			//ST7735_SetTextColor(&lcd, bgColor, fgColor);
			ST7735_FillRoundRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, 3, GREEN);
			ST7735_SetTextColor(&lcd, BLACK, GREEN);
		}

		//draw the text:
		sprintf(lbl, "IN %u", n+1);

		uint16_t tmpTxtW = ST7735_GetTextWidth(&lcd, lbl);
		uint16_t tmpTxtH = ST7735_GetTextHeight(&lcd, lbl, false);
		ST7735_SetCursor(&lcd, tmpLeft + (tmpW / 2) - (tmpTxtW / 2) , (tmpTop + (tmpH/2) ) + (tmpTxtH / 2));
		ST7735_Print(&lcd, lbl);

		//advance to the next position:
		tmpLeft += tmpW + tmpSpace;
	}
}
void SysStatus_UpdateOutputsDisplay(uint8_t newVals){
	uint8_t tmpW = 20;
	uint8_t tmpSpace = 2;
	uint8_t tmpTop = 60;
	uint8_t tmpH = 14;
	uint8_t tmpLeft = 0; //159 - ((2 * tmpW) + tmpSpace);

	char lbl[7][4] = {"W","Y","G","W2","Y2","AUX","REL"};

	for(uint8_t n=0; n<7; n++){
		if(n==5) tmpW = 24;
		if(n==6) tmpW = 23;
		uint8_t curVal = newVals & (1 << n);
		//Draw the box and set text color:
		if(curVal == 0){ //curVal = 0
			ST7735_FillRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, bgColor);
			ST7735_DrawRoundRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, 3, fgColor);
			ST7735_SetTextColor(&lcd, fgColor, bgColor);
		}else{	//curVal = 1
			//ST7735_FillRoundRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, 3, fgColor);
			//ST7735_SetTextColor(&lcd, bgColor, fgColor);
			ST7735_FillRoundRect(&lcd, tmpLeft, tmpTop, tmpW, tmpH, 3, RED);
			ST7735_SetTextColor(&lcd, WHITE, RED);
		}

		//draw the text:
		//sprintf(lbl, "IN %u", n+1);

		uint16_t tmpTxtW = ST7735_GetTextWidth(&lcd, lbl[n]);
		uint16_t tmpTxtH = ST7735_GetTextHeight(&lcd, lbl[n], false);
		ST7735_SetCursor(&lcd, tmpLeft + (tmpW / 2) - (tmpTxtW / 2) , (tmpTop + (tmpH/2) ) + (tmpTxtH / 2));
		ST7735_Print(&lcd, lbl[n]);

		//advance to the next position:
		tmpLeft += tmpW + tmpSpace;
	}
}

void DrawTempLabels(){
	uint8_t x=0, y=20, yInterval = 12;

	ST7735_SetCursor(&lcd, x+8, y);
	ST7735_SetTextColor(&lcd, fgColor, bgColor);
	ST7735_Print(&lcd, "CPU:");

	ST7735_SetCursor(&lcd, x, y + (1*yInterval));
	ST7735_Print(&lcd, "Board:");
	ST7735_SetCursor(&lcd, x+8, y + (2*yInterval));
	ST7735_Print(&lcd, "RT 1:");
	ST7735_SetCursor(&lcd, x+6, y + (3*yInterval));
	ST7735_Print(&lcd, "RT 2:");
	ST7735_SetCursor(&lcd, 90, 50);
	ST7735_Print(&lcd, "Amps:");

}

void SysStatus_UpdateTempDisplay(uint8_t idx, float newTemp){
	DrawTempLabels();

	uint8_t x=38, y=20, yInterval = 12, xErase=40;
	char chrText[7] = {0};
	switch(idx){
	case 0:	 //Cpu temp
		//x=0; y=20;
		sprintf(chrText, "%0.0f", newTemp);
		break;
	case 1:  //Onboard thermistor
		y += (1 * yInterval);
		sprintf(chrText, "%0.1f", newTemp);
		break;
	case 2:  // remote thermistor 1
		y += (2 * yInterval);
		sprintf(chrText, "%0.1f", newTemp);
		break;
	case 3:  // remote thermistor 2
		y += (3 * yInterval);
		sprintf(chrText, "%0.1f", newTemp);
		break;
	case 4:  // current transformer
		x = 126, y = 50;
		sprintf(chrText, "%0.1f", newTemp);
		break;
	default:
		return;
	}

	//if(!(idx == 1 || idx== 4)) return;
	ST7735_FillRect(&lcd, x, y-7, xErase, 7, bgColor);
	ST7735_SetCursor(&lcd, x, y);
	ST7735_Print(&lcd, chrText);


}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	lcd.dmaWriteActive = 0;
	ST7735_Unselect(&lcd);
}
