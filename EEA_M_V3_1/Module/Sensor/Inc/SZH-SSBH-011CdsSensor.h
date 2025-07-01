/**
 * @file SZH-SSBH-011CdsSensor.h
 * @brief CDS 조도 센서 모듈 헤더 파일
 *      Author: PARK JANG HO
 */

#ifndef CDSSENSOR_H_
#define CDSSENSOR_H_

#include "main.h"
#include <stdbool.h>
#include "adc.h"
/*----------------------[ CDS 센서 설정값 정의 ]----------------------*/

/** @brief CDS 센서가 연결된 ADC 채널 */
#define CDS_SENSOR_ADC_CHANNEL   ADC_CHANNEL_15

/** @brief CDS 센서 유효 범위 - 최소값 (이하이면 오류) */
#define CDS_SENSOR_ERROR_MIN     400

/** @brief CDS 센서 유효 범위 - 최대값 (이상이면 오류) */
#define CDS_SENSOR_ERROR_MAX     3600

/** @brief CDS 센서 측정 재시도 횟수 */
#define CDS_SENSOR_RETRY_LIMIT   3

/** @brief ADC 샘플링 횟수 (평균 필터링용) */
#define CDS_SENSOR_SAMPLE_COUNT  5

/** @brief ADC 변환 대기 시간 (단위: ms) */
#define CDS_SENSOR_ADC_TIMEOUT   100

/*----------------------[ CDS 센서 함수 선언 ]----------------------*/
/**
 * @brief CDS 센서 모듈 초기화
 * @details 내부 상태 초기화 및 초기 에러 상태 클리어
 */
void InitCdsSensor(void);
/**
 * @brief CDS 센서 값 평균 측정
 * @return 평균 ADC 값 (0 ~ 4095)
 */
uint32_t GetCdsLevel(void);

/**@brief CDS 센서 오류 여부 검사*/
bool DetectLedFaultByCds(void);
#endif /* CDSSENSOR_H_ */
