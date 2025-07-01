/**
 * @file LedSensor.c
 * @brief LED 제어
 *      Author: PARK JANG HO
 */

#include <LedSensor.h>
#include <stdio.h>

/*----------------------[ LED 초기화 ]----------------------*/
/**
 * @brief LED 핀 초기화
 * @details GPIO를 출력 모드로 설정
 */
void InitLedSensor(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = LED_CONTROL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init(LED_CONTROL_PORT, &GPIO_InitStruct);
}

/*----------------------[ LED 상태 설정 ]----------------------*/
/**
 * @brief LED 상태 직접 제어
 * @param on true = ON, false = OFF
 */
void SetLedState(bool on) {
	HAL_GPIO_WritePin(LED_CONTROL_PORT, LED_CONTROL_PIN,
			on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/*----------------------[ LED ON/OFF 함수 ]----------------------*/
/**
 * @brief LED ON
 */
void TurnOnLed(void) {
	SetLedState(true);
}

/**
 * @brief LED OFF
 */
void TurnOffLed(void) {
	SetLedState(false);
}

/*----------------------[ LED 상태 확인 ]----------------------*/
/**
 * @brief 현재 LED 상태 반환
 * @return true = 켜짐, false = 꺼짐
 */
bool GetLedState(void) {
	return HAL_GPIO_ReadPin(LED_CONTROL_PORT, LED_CONTROL_PIN) == GPIO_PIN_SET;
}
/*----------------------[ 센서 에러 표시 ]----------------------*/
/**
 * @brief 센서 에러 시 깜빡임으로 알림 표시
 * @details LED를 지정 횟수만큼 깜빡여 사용자에게 에러 발생 알림
 */
void HandleSensorError(void) {
	for (int i = 0; i < ERROR_BLINK_COUNT; i++) {
		TurnOnLed();
		HAL_Delay(200);
		TurnOffLed();
		HAL_Delay(200);
	}
}
