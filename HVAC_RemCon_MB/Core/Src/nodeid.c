//-- nodeid.c --//
#include "nodeid.h"
#include "main.h"  // For GPIO defs

uint8_t NodeId_Get(void) {
  uint8_t nodeid = 0;
  if (HAL_GPIO_ReadPin(NODEID_1_GPIO_Port, NODEID_1_Pin) == GPIO_PIN_SET) nodeid |= 0x01;
  if (HAL_GPIO_ReadPin(NODEID_2_GPIO_Port, NODEID_2_Pin) == GPIO_PIN_SET) nodeid |= 0x02;
  if (HAL_GPIO_ReadPin(NODEID_4_GPIO_Port, NODEID_4_Pin) == GPIO_PIN_SET) nodeid |= 0x04;
  if (HAL_GPIO_ReadPin(NODEID_8_GPIO_Port, NODEID_8_Pin) == GPIO_PIN_SET) nodeid |= 0x08;
  return nodeid;
}
