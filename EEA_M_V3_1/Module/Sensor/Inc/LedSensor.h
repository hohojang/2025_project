/**
 * @file LedSensor.h
 * @brief LED 제어 모듈 헤더
 *      Author: PARK JANG HO
 */

#ifndef LED_SENSOR_H_
#define LED_SENSOR_H_

#include "main.h"
#include <stdbool.h>

#define LED_CONTROL_PORT GPIOA
#define LED_CONTROL_PIN  GPIO_PIN_8
#define ERROR_BLINK_COUNT 2

/*----------------------[ LED 제어 함수 (CMD_LED_CTRL_REQ / RES) ]----------------------*/

void InitLedSensor(void);
void SetLedState(bool on);
void TurnOnLed(void);
void TurnOffLed(void);
bool GetLedState(void);  // CMD_LIGHT_STATE_REQ 응답 시 사용

/*----------------------[ LED 에러 표시 함수 (CMD_LED_FAULT_REQ) ]----------------------*/

void HandleSensorError(void);

#endif /* LED_SENSOR_H_ */
