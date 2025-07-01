/**
 * @file SZH-SSBH-011CdsSensor.c
 * @brief CDS 조도 센서 모듈
 *      Author: PARK JANG HO
 * @details
 * - ADC를 통해 조도 값을 측정하며, 정의서 기준의 유효 범위 벗어날 경우 오류로 간주
 * - 평균값 필터링을 통해 노이즈를 완화
 * - 오류 상태 변화 시에만 패킷 전송 수행
 */
#include <stdio.h>
#include "LedSensor.h"
#include <Sensor.h>
#include <SZH-SSBH-011CdsSensor.h>

#define DEFAULT_CDS_SENSOR_MIN 200
#define DEFAULT_CDS_SENSOR_MAX 3000

static uint16_t cds_error_min = DEFAULT_CDS_SENSOR_MIN;
static uint16_t cds_error_max = DEFAULT_CDS_SENSOR_MAX;
/*----------------------[ 전역 변수 ]----------------------*/
extern ADC_HandleTypeDef hadc1;       ///< ADC1 핸들
static bool lastErrorState = false;   ///< 이전 오류 상태 (중복 보고 방지)
/*----------------------[ 센서 초기화 ]----------------------*/
/**
 * @brief CDS 센서 초기화
 * @details 내부 오류 상태 초기화
 */
void InitCdsSensor(void) {
	lastErrorState = false;
}
/*----------------------[ 조도 센서 측정 ]----------------------*/

/**
 * @brief 조도 센서 값 측정
 * @details ADC N회 샘플링 평균값을 반환
 * @return 평균 ADC 값 (0 ~ 4095)
 */
uint32_t GetCdsLevel(void) {
	uint32_t sum = 0;

	for (int i = 0; i < CDS_SENSOR_SAMPLE_COUNT; i++) {
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, CDS_SENSOR_ADC_TIMEOUT);
		sum += HAL_ADC_GetValue(&hadc1);
	}

	return sum / CDS_SENSOR_SAMPLE_COUNT;
}
/*----------------------[ 센서 오류 검사 ]----------------------*/
/**
 * @brief CDS 조도 값이 정상 범위를 벗어났는지 확인
 * @return true = 오류 감지됨, false = 정상 범위
 */
bool IsCdsLevelOutOfRange(void) {
	uint32_t level = GetCdsLevel();
	return (level < cds_error_min || level > cds_error_max);
}
/**
 * @brief 전등이 켜졌을 때 CDS 센서 값이 비정상인지 확인 (LED 고장 탐지)
 * @return true = 고장 감지, false = 정상
 */
bool DetectLedFaultByCds(void) {
	return (GetLedState() == LED_ON && IsCdsLevelOutOfRange());
}
