/**
 * @file protocol.h
 * @brief EEA 프로토콜 기반 헤더
 *      Author: PARK JANG HO
 */
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "main.h"
#include "stdint.h"
#include <stdbool.h>

void ProcessLoRaReception(void);
void SendPirOffTimeResponse(uint8_t dstId);
/* ======================= [ 프로토콜 기본 정의 ] ======================= */
#define EEA_STX  0xFE  ///< 패킷 시작 바이트
#define EEA_ETX  0xEF  ///< 패킷 종료 바이트

#define EEA_MAX_DATA_SIZE     8 ///< 최대 데이터 길이

#define EEA_MAX_PACKET_SIZE   (6 + EEA_MAX_DATA_SIZE + 2)  ///< STX~ETX 포함 최대 패킷 크기
#define EEA_MIN_PACKET_SIZE   8   ///< 최소 유효 패킷 크기 (STX ~ ETX)
/* ======================= [ EEA-M & EEA-RTU ] ======================= */
/** 전등 제어 (CS → RTU → M) */
#define CMD_LED_CTRL_RES              0x01  ///< [RTU→M]
/** 전등 상태 변화 (M → RTU) */
#define CMD_LED_CTRL_REQ              0x02  ///< [M→RTU]
/** PIR 감지 */
#define CMD_PIR_MOVE_REQ              0x03  ///< [M→RTU]
/** 전등 불량 */
#define CMD_LED_FAULT_REQ             0x04  ///< [M→RTU]
/** 전등 불량 확인 응답 */
#define CMD_LED_FAULT_RES             0x64  ///< [RTU→M]
/** PIR 꺼짐 시간 제어 요청(CS → RTU → M) */
#define CMD_PIR_OFF_DELAY_REQ         0xA1  ///< [RTU→M]
/** PIR 꺼짐 시간 제어 응답 */
#define CMD_PIR_OFF_DELAY_RES         0x1A  ///< [M→RTU]

/* ======================= [ EEA 패킷 구조체 정의 ] ======================= */
/**
 * @brief EEA 통신 패킷 구조체
 */
typedef struct {
	uint8_t stx;                          ///< 패킷 시작 바이트
	uint8_t srcId;                        ///< 송신 디바이스 ID
	uint8_t dstId;                        ///< 수신 디바이스 ID
	uint8_t blockId;                      ///< 송신 디바이스 블록 ID
	uint8_t command;                      ///< 명령어 코드
	uint8_t length;                       ///< 데이터 필드 길이
	uint8_t data[EEA_MAX_DATA_SIZE];      ///< 데이터
	uint8_t checksum;                     ///< 체크섬
	uint8_t etx;                          ///< 패킷 종료 바이트
} EEAPacket_t;

int SerializeEEAPacket(uint8_t *dest, const EEAPacket_t *pkt);

/* ======================= [ 함수 선언 ] ======================= */

/**
 * @brief EEA 패킷 송신 함수
 * @param pkt 송신할 패킷 포인터
 * @return HAL 상태값
 */
HAL_StatusTypeDef SendEEAPacket(const EEAPacket_t *pkt);
#endif // __PROTOCOL_H__
