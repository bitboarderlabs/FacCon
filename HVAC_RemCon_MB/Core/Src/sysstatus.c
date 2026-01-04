#include "sysstatus.h"
#include "main.h"

static GPIO_TypeDef* 	StatusLedPort = 0;
static uint16_t			StatusLedPin = 0;
static GPIO_PinState	StatusLedOffState = 0;
static GPIO_PinState	StatusLedOnState = 0;

static GPIO_TypeDef* 	CommLedPort = 0;
static uint16_t			CommLedPin = 0;
static GPIO_PinState	CommLedOffState = 0;
static GPIO_PinState	CommLedOnState = 0;

static GPIO_TypeDef* 	AlarmLedPort = 0;
static uint16_t			AlarmLedPin = 0;
static GPIO_PinState	AlarmLedOffState = 0;
static GPIO_PinState	AlarmLedOnState = 0;

static GPIO_TypeDef* 	LcdBLPort = 0;
static uint16_t			LcdBLPin = 0;
static GPIO_PinState	LcdBlOffState = 0;
static GPIO_PinState	LcdBlOnState = 0;

static SPI_HandleTypeDef* hLcdSpi = 0;
static GPIO_TypeDef* 	LcdRestPort = 0;
static uint16_t			LcdResetPin = 0;
static GPIO_TypeDef* 	LcdCsPort = 0;
static uint16_t			LcdCsPin = 0;
static GPIO_TypeDef* 	LcdAoPort = 0;
static uint16_t			LcdAoPin = 0;

static uint8_t			UseLcd = 0;

static SystemState_t current_state = SYSTEM_STATE_BOOT;
static uint32_t system_blink_tick = 0;
static uint32_t comms_blink_start = 0;
static uint8_t comms_led_active = 0;

//void LEDs_Init(void)
//{
//    HAL_GPIO_WritePin(LED_STATUS_PORT, LED_STATUS_PIN, LED_OFF_STATE);   // System LED off
//    HAL_GPIO_WritePin(LED_COMM_PORT, LED_COMM_PIN, LED_OFF_STATE);   // Comms LED off
//    HAL_GPIO_WritePin(LED_ALARM_T1C3_GPIO_Port, LED_ALARM_T1C3_Pin, LED_OFF_STATE);   // LD3 off (if used later)
//
//    current_state = SYSTEM_STATE_BOOT;
//    system_blink_tick = HAL_GetTick();
//    comms_blink_start = 0;
//    comms_led_active = 0;
//}

void SysStatus_Init_WithLCD(GPIO_TypeDef* statusLedPort, uint16_t statusLedPin, GPIO_PinState statusLedOnState,
		GPIO_TypeDef* commLedPort, uint16_t commLedPin, GPIO_PinState commLedOnState,
		GPIO_TypeDef* alarmLedPort, uint16_t alarmLedPin, GPIO_PinState alarmLedOnState,
		GPIO_TypeDef* lcdBacklightPort, uint16_t lcdBacklightPin, GPIO_PinState lcdBlOnState,
		SPI_HandleTypeDef* hlcdSpiHandle,
		GPIO_TypeDef* lcdCsPort, uint16_t lcdCsPin, GPIO_TypeDef* lcdAoPort, uint16_t lcdAoPin,
		GPIO_TypeDef* lcdResetPort, uint16_t lcdResetPin){

	StatusLedPort = statusLedPort;
	StatusLedPin = statusLedPin;
	StatusLedOnState = (statusLedOnState == GPIO_PIN_SET ? GPIO_PIN_SET : GPIO_PIN_RESET);
	StatusLedOffState = (statusLedOnState == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET);

	CommLedPort = commLedPort;
	CommLedPin = commLedPin;
	CommLedOnState = (commLedOnState == GPIO_PIN_SET ? GPIO_PIN_SET : GPIO_PIN_RESET);
	CommLedOffState = (commLedOnState == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET);

	AlarmLedPort = alarmLedPort;
	AlarmLedPin = alarmLedPin;
	AlarmLedOnState = (commLedOnState == GPIO_PIN_SET ? GPIO_PIN_SET : GPIO_PIN_RESET);
	AlarmLedOffState = (commLedOnState == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET);

	LcdBLPort = lcdBacklightPort;
	LcdBLPin = lcdBacklightPin;
	LcdBlOnState = (lcdBlOnState == GPIO_PIN_SET ? GPIO_PIN_SET : GPIO_PIN_RESET);
	LcdBlOffState = (lcdBlOnState == GPIO_PIN_SET ? GPIO_PIN_RESET : GPIO_PIN_SET);

	if(hLcdSpi > 0){
		UseLcd = 1;
		hLcdSpi = hlcdSpiHandle;
	}else{
		UseLcd = 0;
		hLcdSpi = 0;
	}

	LcdRestPort = lcdResetPort;
	LcdResetPin = lcdResetPin;
	LcdCsPort = lcdCsPort;
	LcdCsPin = lcdCsPin;
	LcdAoPort = lcdAoPort;
	LcdAoPin = lcdAoPin;

	HAL_GPIO_WritePin(StatusLedPort, StatusLedPin, StatusLedOffState);   // System LED off
	HAL_GPIO_WritePin(CommLedPort, CommLedPin, CommLedOffState);   // Comms LED off
	HAL_GPIO_WritePin(AlarmLedPort, AlarmLedPin, AlarmLedOffState);   // LD3 off (if used later)

	current_state = SYSTEM_STATE_BOOT;
	system_blink_tick = HAL_GetTick();
	comms_blink_start = 0;
	comms_led_active = 0;
}

void SysStatus_Init_NoLCD(GPIO_TypeDef* statusLedPort, uint16_t statusLedPin, GPIO_PinState statusLedOnState,
		GPIO_TypeDef* commLedPort, uint16_t commLedPin, GPIO_PinState commLedOnState,
		GPIO_TypeDef* alarmLedPort, uint16_t alarmLedPin, GPIO_PinState alarmLedOnState){

	SysStatus_Init_WithLCD(statusLedPort, statusLedPin, statusLedOnState,
			commLedPort, commLedPin, commLedOnState, alarmLedPort, alarmLedPin, alarmLedOnState,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void LEDs_SetSystemState(SystemState_t state)
{
    if (current_state != state) {
        current_state = state;
        if (state == SYSTEM_STATE_BOOT) {
            HAL_GPIO_WritePin(StatusLedPort, StatusLedPin, StatusLedOffState);
        }
        // When switching to STABLE, next blink will handle the toggle
    }
}

void LED_Comms_Trigger(void)
{
    if (!comms_led_active) {
        HAL_GPIO_WritePin(CommLedPort, CommLedPin, CommLedOnState);
        comms_blink_start = HAL_GetTick();
        comms_led_active = 1;
    }
}

void LEDs_Process(void)
{
    // Comms LED: 10 ms pulse
    if (comms_led_active) {
        if (HAL_GetTick() - comms_blink_start >= 10) {
            HAL_GPIO_WritePin(CommLedPort, CommLedPin, CommLedOnState);
            comms_led_active = 0;
        }
    }

    // System LED: only active in STABLE state
    if (current_state == SYSTEM_STATE_STABLE) {
        if (HAL_GetTick() - system_blink_tick >= 4000) {  // Toggle every 4 seconds
            HAL_GPIO_TogglePin(StatusLedPort, StatusLedPin);
            system_blink_tick = HAL_GetTick();
        }
    }
}
