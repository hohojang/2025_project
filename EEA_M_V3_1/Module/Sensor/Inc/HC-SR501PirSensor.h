/**
 * @file HC-SR501PirSensor.h
 * @brief PIR 센서(HC-SR501) 헤더 파일
 *      Author: PARK JANG HO
 */

#ifndef PIRSENSOR_H_
#define PIRSENSOR_H_

#include "main.h"
#include <stdbool.h>

/*----------------------[ PIR 핀 정의 ]----------------------*/

#define PIR_SENSOR_PORT GPIOC
#define PIR_SENSOR_PIN  GPIO_PIN_6

/*----------------------[ 함수 선언 (정의서 기준) ]----------------------*/

/**
 * @brief PIR 센서 초기화
 */
void InitPirSensor(void);

/**
 * @brief PIR 감지 여부 반환 (CMD_PIR_MOVE_REQ)
 * @return 감지되면 true, 아니면 false
 */
bool IsPirstate(void);

#endif /* PIRSENSOR_H_ */
