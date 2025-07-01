/**
 * @file HC-SR501PirSensor.c
 * @brief PIR 인체 감지 센서 처리 모듈
 *      Author: PARK JANG HO
 */

#include "../../../Module/Sensor/Inc/HC-SR501PirSensor.h"

/*----------------------[ 센서 핀 초기화 ]----------------------*/
/**
 * @brief PIR 센서 핀 초기화
 * @details 입력 모드, 풀업/풀다운 없음
 */
void InitPirSensor(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOC_CLK_ENABLE();

	GPIO_InitStruct.Pin = PIR_SENSOR_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;

	HAL_GPIO_Init(PIR_SENSOR_PORT, &GPIO_InitStruct);
}

/*----------------------[ 감지 상태 반환 ]----------------------*/
/**
 * @brief PIR 센서 감지 여부 반환
 * @return true = 감지됨, false = 감지 안됨
 */
bool IsPirstate(void) {
	return HAL_GPIO_ReadPin(PIR_SENSOR_PORT, PIR_SENSOR_PIN) == GPIO_PIN_SET;
}
