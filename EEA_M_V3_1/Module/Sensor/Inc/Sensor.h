/**
 * @file Sensor.h
 * @brief 센서 통합 제어 및 상태 패킷 전송 헤더
 * @author PARK JANG HO
 */

#ifndef SENSOR_H_
#define SENSOR_H_

#include <E220-900T22D.h>
#include <stdint.h>
#include <stdbool.h>

#include "protocol.h"

// -----------------------------------------------------------------------------
// [1] 시스템 상수 정의
// -----------------------------------------------------------------------------

#define SENSOR_REPORT_INTERVAL_MS     10000   ///< 주기적 센서 상태 보고 간격 (ms)

#define PIR_HOLD_TIME_MS              10000   ///< PIR 감지 후 LED 유지 시간 (기본 10초 → 시험 시 600000)
#define MSB_SHIFT                     8       ///< 16비트 주소 상위 바이트 시프트
#define EEA_MIN_PACKET_LEN            8       ///< 최소 EEA 패킷 길이 (STX~ETX 포함)

#define CDS_ERROR_HOLD_TIME_MS        4000    ///< CDS 오류 상태 유지 시간
#define CDS_RETRY_TIMEOUT_MS          5000    ///< CDS 고장 판단 전 대기 시간
#define PIR_STATE_CONFIRM_DELAY_MS    500     ///< PIR 감지 확정 대기 시간
#define MANUAL_OVERRIDE_TIMEOUT_MS    3000    ///< 자동 복귀까지의 수동 중단 시간 (ms)

// -----------------------------------------------------------------------------
// [2] 센서/LED 상태 코드
// -----------------------------------------------------------------------------

#define PIR_DETECTED                  1
#define PIR_NOT_DETECTED              0

#define LED_OFF                       0
#define LED_ON                        1

#define CDS_NO_ERROR                  0
#define CDS_ERROR                     1

///< CDS 오류 상태 식별 매크로
#define CDS_ERROR_FLASH_ID    0xA5A5A5A5
#define CDS_NO_ERROR_FLASH_ID 0x00000000
// -----------------------------------------------------------------------------
// [3] 전역 상태 변수 (외부 접근 가능)
// -----------------------------------------------------------------------------

extern volatile EEAPacket_t latestRxPacket;    ///< 마지막 수신 패킷 정보
extern volatile EEAPacket_t latestTxPacket;    ///< 마지막 송신 패킷 정보
extern volatile bool ackReceivedFlag;          ///< ACK 응답 수신 여부

// PIR 관련
extern uint32_t pirHoldDuration;
extern uint32_t lastPirDetectionTime;
extern uint8_t  currentPirStatus;
extern uint8_t  lastPirStatus;
extern uint32_t pirChangeStartTime;
extern bool     pirChangePending;

// LED 상태 보고 추적
extern uint8_t  lastReportedLedStatus;

// CDS 고장 상태 추적
extern uint8_t  lastCdsError;
extern uint32_t cdsErrorStartTime;
extern bool     cdsErrorPending;


uint8_t CalculateChecksum(const uint8_t *raw, uint8_t length);
// -----------------------------------------------------------------------------
// [4] 센서 제어/처리 함수
// -----------------------------------------------------------------------------

/**
 * @brief CDS / PIR / LED 센서 전체 초기화
 */
void InitAllSensors(void);

/**
 * @brief 센서 감지 및 상태 변화에 따른 처리
 */
void ProcessAllSensors(void);

/**
 * @brief PIR 감지 시 동작 처리
 */
void PirDetected(uint32_t now);

/**
 * @brief PIR 유지 시간이 만료되었을 때 처리
 */
void PirHoldTimeout(uint32_t now);

// -----------------------------------------------------------------------------
// [5] 명령어 처리 핸들러 (RTU 요청에 대한 처리)
// -----------------------------------------------------------------------------

void LedControlCommand(const uint8_t *data, uint8_t len);
void PirDelayCommand(const uint8_t *data, uint8_t len, uint8_t srcId);

// -----------------------------------------------------------------------------
// [6] 센서 상태 전송 / 보고 함수
// -----------------------------------------------------------------------------
/**
 * @brief 전등 상태 보고 (0x01 = ON, 0x00 = OFF)
 */
void SendLedStateReport(uint8_t ledStatus);

/**
 * @brief PIR 감지 상태 보고
 */
void SendPirStatusPacket(bool detected);

/**
 * @brief CDS 기반 고장 상태 또는 복구 상태 보고
 * @param fault true = 고장 발생(0x01), false = 고장 복구(0x00)
 */
void SendLightFail(bool fault);

void SendEEADataPacket(uint8_t dstId, uint8_t command, uint8_t *payload, uint8_t length);
/**
 * @brief CDS 상태 기반 전등 고장 여부 판단 및 전송
 */
void CheckLightFail(bool cdsFault, uint32_t now);

// -----------------------------------------------------------------------------
// [7] 외부 접근 가능한 센서 상태 확인 함수
// -----------------------------------------------------------------------------

uint32_t GetCdsLevel(void);
uint8_t GetLedFaultStatus(void);

#endif /* SENSOR_H_ */
