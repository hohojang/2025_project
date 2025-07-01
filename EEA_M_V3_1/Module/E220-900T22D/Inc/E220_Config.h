/*
 * E220_Config.h
 *
 * E220-900T22D 모듈 설정용 매크로 및 구조체 정의 파일
 *      Author: PARK JANG HO
 *  역할
 * - 모듈 설정 시 사용되는 상수(모드, 채널, 전력, Baudrate 등)를 일관성 있게 정의
 * - 사용자 설정을 하나의 구조체(E220_Config)로 관리할 수 있게 제공
 */
#include <stdint.h>
#include <stdio.h>

#ifndef E220_900T22D_E220_CONFIG_H_
#define E220_900T22D_E220_CONFIG_H_

// =============================
// 전송 모드 (AT+TRANS)
// =============================
#define E220_TRANS_TRANSPARENT   0   // 투명 모드
#define E220_TRANS_FIXED         1   // 고정 모드

// =============================
// 전송 출력 파워 (AT+POWER)
// =============================
#define E220_POWER_22DBM         0   // 최대 출력
#define E220_POWER_17DBM         1
#define E220_POWER_13DBM         2
#define E220_POWER_10DBM         3   // 최소 출력

// =============================
// UART 통신 속도
// =============================
#define E220_BAUDRATE_1200       0x00
#define E220_BAUDRATE_2400       0x01
#define E220_BAUDRATE_4800       0x02
#define E220_BAUDRATE_9600       0x03
#define E220_BAUDRATE_19200      0x04
#define E220_BAUDRATE_38400      0x05
#define E220_BAUDRATE_57600      0x06
#define E220_BAUDRATE_115200     0x07


// =============================
// 기능 활성화 여부
// =============================
#define E220_FEATURE_DISABLE     0
#define E220_FEATURE_ENABLE      1

// =============================
// 패킷 및 명령어 상수
// =============================
#define DEST_ID_LENGTH           3        // 목적지 ID 길이 (ADDH, ADDL, CH)
#define REG_CMD_HEADER_LEN       3        // C0/C1/C2 + StartAddr + Len

#define E220_UART_TX_TIMEOUT     500      // UART Transmit timeout in ms
#define E220_UART_RX_TIMEOUT     2000     // UART Receive timeout in ms
#define E220_AT_MODE_DELAY       100      // 설정 모드 진입 후 대기 시간
#define E220_AT_RESPONSE_DELAY   10       // AT 응답 대기 시간 (송신 직후)

typedef enum {
	LORA_MODE_NORMAL  = 0, ///< 투명 전송 모드
	LORA_MODE_WOR_TX  = 1, ///< WOR 송신 모드
	LORA_MODE_WOR_RX  = 2, ///< WOR 수신 모드
	LORA_MODE_CONFIG  = 3, ///< 설정 모드 (AT 명령, 슬립 포함)
	LORA_MODE_UNKNOWN = -1  ///< 초기값 또는 알 수 없음
} LoRaMode_t;

typedef enum {
	MODULE_TYPE_E220, MODULE_TYPE_E22
} ModuleType_t;

// =============================
// 사용자 설정 구조체 정의 [설정용]
// =============================
typedef struct {
	uint16_t addr;            ///< LoRa 주소 (ADDH+ADDL)
	uint8_t channel;          ///< 통신 채널 (0x00 ~ 0x53)
	uint8_t transMode;        ///< 전송 모드 (Transparent / Fixed)
	uint8_t power;            ///< 출력 전력 (10/13/17/22 dBm)
	uint32_t baudrate;        ///< UART 보드레이트
	uint8_t fec;              ///< FEC 사용 여부
	uint8_t rssi;             ///< RSSI 반환 여부
	uint8_t wor;              ///< WOR 기능 활성화 여부
	uint8_t defaultMode;      ///< 초기 진입 모드 (TRANS_MODE 등)
	ModuleType_t moduleType;

} E220_Config;
/* ======================= [ 시스템 상수 ] ======================= */
///< [Device ID 정의]
#define EEA_M_ID         		      0x02  ///< 현재 디바이스 ID
#define EEA_RTU_ID            		  0xFA  ///< 수신 대상 RTU ID
// [LoRa Fixed Mode 상위 주소 구성용]
#define CHANNEL_ID        		      0x01  ///< 현재 디바이스 채널 ID
#define DST_HIGH_ID      		      0x00  ///< 상위 주소 ID
// [센서/블록 구분 ID]
#define BLOCK_ID_SENSOR   		      0x01  ///< 센서 블록 ID
// =============================
// 사용자 정의 기본 설정 매크로
// =============================
#define E220_LORA_ADDR        0x02
#define E220_LORA_CHANNEL     0x01
#define E220_LORA_TRANS_MODE  E220_TRANS_FIXED
#define E220_LORA_POWER       E220_POWER_22DBM
#define E220_LORA_BAUDRATE    E220_BAUDRATE_9600
#define E220_LORA_FEC         E220_FEATURE_ENABLE
#define E220_LORA_RSSI        E220_FEATURE_ENABLE
#define E220_LORA_WOR         E220_FEATURE_DISABLE
#define E220_LORA_MODE        TRANS_MODE
#define E220_LORA_MODULE_TYPE MODULE_TYPE_E220

///< M0 , M1 핀 설정
#define LORA_GPIO_PORT_M0        GPIOB
#define LORA_GPIO_PIN_M0         GPIO_PIN_1
#define LORA_GPIO_PORT_M1        GPIOA
#define LORA_GPIO_PIN_M1         GPIO_PIN_9

#endif /* E220_900T22D_E220_CONFIG_H_ */
