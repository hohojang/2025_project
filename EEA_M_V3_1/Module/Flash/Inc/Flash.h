#ifndef FLASH_FLASH_H_
#define FLASH_FLASH_H_

#include "stm32l4xx_hal.h"
#include <stdint.h>

// FLASH_PAGE_SIZE 제거 (HAL이 제공)
#define FLASH_USER_START_ADDR  0x08060000U  // Page 192 시작 주소
#define FLASH_USER_END_ADDR    0x0807FFFFU  // Page 192 끝 주소

typedef enum {
    LED1_FAULT = (1 << 0),
    LED2_FAULT = (1 << 1),
    LED3_FAULT = (1 << 2)
} LedFaultStatus;

HAL_StatusTypeDef SaveFaultStatus(uint32_t status);
uint32_t ReadFaultStatus(void);

#endif /* FLASH_FLASH_H_ */
