/*
 * E220_900T22D.c
 *
 * E220_900T22D LoRa 모듈 드라이버
 *      Author: PARK JANG HO
*/
#include <E220-900T22D.h>
#include <E220_Config.h>
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <main.h>
#include <NNFQueue.h>

/// @brief M0 핀 현재 상태
volatile uint8_t m0PinState = 0;

/// @brief M1 핀 현재 상태
volatile uint8_t m1PinState = 0;

extern UART_HandleTypeDef huart3;
extern CircularQueue loRaRxQueue;
extern volatile uint8_t rx_byte;
extern uint8_t gDstInfo[3];

/// @brief 기본 E220 모듈 인스턴스
E220 myLoRa;
/**
 * @brief LoRa 구조체 기본값 생성 (통신 설정 제거)
 *
 * @details
 * - 이 함수는 하드웨어 연결 이전의 초기 상태를 나타냄
 * - 모든 포인터 및 설정 필드를 명시적으로 비활성화 상태로 초기화함
 * - 이후 ConfigureLoRaHardware(), ApplyLoRaParameters()로 실제 설정 적용 필요
 *
 * @return 초기화된 E220 구조체 (주소/채널/통신 설정은 아직 적용되지 않음)
 */
E220 NewLoRa(void) {
    E220 lora;
    // ------------------------------[ 하드웨어 연결 필드 초기화 ]------------------------------ //
    lora.pHuart = NULL;          ///< UART 포인터 초기화 (연결 안됨)
    lora.M0Port = NULL;          ///< M0 제어 핀 포트 미설정
    lora.M0Pin  = 0;             ///< M0 핀 번호 미설정
    lora.M1Port = NULL;          ///< M1 제어 핀 포트 미설정
    lora.M1Pin  = 0;             ///< M1 핀 번호 미설정
    // ------------------------------[ 기타 필드 ]------------------------------ //
    lora.current_mode = -1;                 ///< 아직 모드 설정 안 됨
    lora.moduleType   = MODULE_TYPE_E220;   ///< 기본 모듈 타입 설정

    return lora;
}
/**
 * @brief LoRa 구조체에 실제 하드웨어 리소스 설정
 * @details
 * - M0/M1 핀과 UART 포인터를 연결해 실제 MCU와의 물리적 연결 정보 설정
 * - 구조체를 LoRa 설정 및 제어에 사용할 준비 상태로 만듦
 */
void ConfigureLoRaHardware(E220 *lora) {
    // --------------------[ 1. UART 포인터 연결 ]-------------------- //
    lora->pHuart = &huart3;

    // --------------------[ 2. M0 핀 설정 ]-------------------- //
    lora->M0Port = LORA_GPIO_PORT_M0;
    lora->M0Pin  = LORA_GPIO_PIN_M0;

    // --------------------[ 3. M1 핀 설정 ]-------------------- //
    lora->M1Port = LORA_GPIO_PORT_M1;
    lora->M1Pin  = LORA_GPIO_PIN_M1;
}
/**
 * @brief 외부 설정 구조체(config)를 LoRa 구조체에 반영
 * @details
 * - 주소/채널 등 통신 파라미터를 LoRa 구조체에 복사하여 구조체 완성
 * - 하드웨어 설정 이후 통신 동작에 필요한 모든 값이 이 함수에서 채워짐
 */
void ApplyLoRaParameters(E220 *lora, const E220_Config *config) {
    // --------------------[ 1. 기본 통신 설정 ]-------------------- //
    lora->Addr      = config->addr;
    lora->Channel   = config->channel;
    lora->Trans     = config->transMode;
    lora->Power     = config->power;
    lora->Baudrate  = config->baudrate;

    // --------------------[ 2. 부가 통신 옵션 ]-------------------- //
    lora->FEC        = config->fec;
    lora->RSSI       = config->rssi;
    lora->WOR        = config->wor;
    lora->defaultMode = config->defaultMode;

    // --------------------[ 3. 모듈 종류 정보 ]-------------------- //
    lora->moduleType = config->moduleType;
}

void E220LoRaEnterConfigMode(E220 *lora) {
    HAL_GPIO_WritePin(lora->M0Port, lora->M0Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(lora->M1Port, lora->M1Pin, GPIO_PIN_SET);
    HAL_Delay(100);  // 안정화 대기
}

void E220LoRaEnterNormalMode(E220 *lora) {
    HAL_GPIO_WritePin(lora->M0Port, lora->M0Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(lora->M1Port, lora->M1Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
}

void E220_SetAddress(E220 *lora, uint16_t address) {
    uint8_t addrBuf[3] = { 0xC0, (address >> 8) & 0xFF, address & 0xFF }; // AT+ADDR
    HAL_UART_Transmit(lora->pHuart, addrBuf, sizeof(addrBuf), HAL_MAX_DELAY);
    HAL_Delay(50);
}

void E220_SetChannel(E220 *lora, uint8_t channel) {
    uint8_t chanBuf[2] = { 0xC1, channel }; // AT+CHAN
    HAL_UART_Transmit(lora->pHuart, chanBuf, sizeof(chanBuf), HAL_MAX_DELAY);
    HAL_Delay(50);
}

void E220_SetTransmitPower(E220 *lora, uint8_t power) {
    // power: 0~3 범위 (00 = 최대출력)
    uint8_t reg0Buf[2] = { 0xC2, (power & 0x03) << 6 };  // AT+REG0
    HAL_UART_Transmit(lora->pHuart, reg0Buf, sizeof(reg0Buf), HAL_MAX_DELAY);
    HAL_Delay(50);
}

void E220_SetAirBaudRate(E220 *lora, uint8_t baudRate) {
    uint8_t reg0Buf[2] = { 0xC3, baudRate & 0x1F };  // AT+REG0 baud만 변경
    HAL_UART_Transmit(lora->pHuart, reg0Buf, sizeof(reg0Buf), HAL_MAX_DELAY);
    HAL_Delay(50);
}

void E220_SetFEC(E220 *lora, uint8_t fec) {
    uint8_t reg0Buf[2] = { 0xC4, (fec & 0x01) << 5 }; // AT+REG0 FEC만 변경
    HAL_UART_Transmit(lora->pHuart, reg0Buf, sizeof(reg0Buf), HAL_MAX_DELAY);
    HAL_Delay(50);
}

void E220_SetFixedMode(E220 *lora) {
    uint8_t reg1Buf[2] = { 0xC5, 0x40 }; // AT+REG1 : Fixed Mode 활성화 (bit6 = 1)
    HAL_UART_Transmit(lora->pHuart, reg1Buf, sizeof(reg1Buf), HAL_MAX_DELAY);
    HAL_Delay(50);
}

/**
 * @brief 최상위 LoRa 모듈 초기화 함수
 */
void InitLoRaModule(void) {
    // [1] LoRa 구조체 기본 초기화
    myLoRa = NewLoRa();

    // [2] 하드웨어 핀 설정
    myLoRa.pHuart = &huart3;
    myLoRa.M0Port = LORA_GPIO_PORT_M0;
    myLoRa.M0Pin  = LORA_GPIO_PIN_M0;
    myLoRa.M1Port = LORA_GPIO_PORT_M1;
    myLoRa.M1Pin  = LORA_GPIO_PIN_M1;
    myLoRa.moduleType = E220_LORA_MODULE_TYPE;

    // [3] 수동 설정 기반으로 RF 세팅은 생략 (RF Tool로 설정 예정)

    // [4] Normal 모드 전환
    E220LoRaEnterNormalMode(&myLoRa);   // M0=0, M1=0
}



/**
 * @brief LoRa 모듈의 모드를 M0/M1 핀을 통해 설정
 *
 * @param lora 설정할 LoRa 인스턴스 포인터
 * @param mode 설정할 LoRa 동작 모드 (NORMAL, WOR_TX, WOR_RX, CONFIG)
 *
 * @details
 * - 모듈 타입(E220 또는 E22)에 맞춰 내부적으로 M0/M1 핀 상태를 계산
 * - 핀 상태 설정 후, 안정적인 모드 전환을 위해 1초 지연을 줌
 * - 현재 모드 상태를 구조체에 기록하고, 디버깅 로그 출력
 */
void LoRaSetMode(E220 *lora, LoRaMode_t mode) {
    /* ------------------------------------------------------------------------
     * [1] M0/M1 핀 상태 계산
     * - 모듈 타입에 따라 지정된 모드에 대한 핀 상태 결정
     * ------------------------------------------------------------------------ */
    GPIO_PinState m0 = GPIO_PIN_RESET;
    GPIO_PinState m1 = GPIO_PIN_RESET;
    /* ------------------------------------------------------------------------
     * [2] M0/M1 핀 상태 설정
     * - 해당 핀 포트/핀 넘버는 lora 구조체에 사전 설정되어 있어야 함
     * ------------------------------------------------------------------------ */
    HAL_GPIO_WritePin(lora->M0Port, lora->M0Pin, m0);
    HAL_GPIO_WritePin(lora->M1Port, lora->M1Pin, m1);
    /* ------------------------------------------------------------------------
     * [3] 안정적인 모드 전환을 위한 지연
     * - 데이터시트 기준: 모드 전환 후 최소 수백 ms 대기 권장
     * ------------------------------------------------------------------------ */
    HAL_Delay(1000);  // 1초 지연 (권장값 또는 안정성 고려)
    /* ------------------------------------------------------------------------
     * [4] 현재 모드 저장
     * ------------------------------------------------------------------------ */
    lora->current_mode = mode;
}

// ============================================================================
// 데이터 송신
// ============================================================================

/**
 * @brief LoRa 패킷 조립
 * @details
 *   - 앞 3바이트는 수신자 주소(High, Low, Channel)
 *   - 그 뒤는 payload (EEA 패킷 등)
 */
int iE220LoRaMakePacket(const uint8_t *uchDstID, const uint8_t *uchData,
                        int iDataLength, uint8_t *uchPaket) {
    // --------------------[ 1. 목적지 주소 3바이트 삽입 ]-------------------- //
    uchPaket[0] = uchDstID[0];  ///< ADDH
    uchPaket[1] = uchDstID[1];  ///< ADDL
    uchPaket[2] = uchDstID[2];  ///< CHAN
    // --------------------[ 2. 데이터 복사 ]-------------------- //
    memcpy(&uchPaket[3], uchData, iDataLength);
    // --------------------[ 3. 전체 패킷 길이 반환 ]-------------------- //
    return 3 + iDataLength;
}
/**
 * @brief LoRa 패킷 송신
 */
void E220LoRaSendPacket(E220 *lora, uint8_t *buffer, size_t len) {
    /* ------------------------------------------------------------------------
     * [1] 현재 모드가 NORMAL이 아니면 NORMAL 모드로 전환
     * - 전환 후 안정화를 위해 200ms 지연
     * ------------------------------------------------------------------------ */
    if (lora->current_mode != LORA_MODE_NORMAL) {
        LoRaSetMode(lora, LORA_MODE_NORMAL);  // M0/M1 핀 설정
        HAL_Delay(200);                       // 모드 전환 안정화 시간
    }
    /* ------------------------------------------------------------------------
     * [2] UART를 통해 패킷 송신
     * - HAL_MAX_DELAY: 블로킹 방식으로 전송 완료 대기
     * ------------------------------------------------------------------------ */
    HAL_UART_Transmit(lora->pHuart, buffer, len, HAL_MAX_DELAY);
    /* ------------------------------------------------------------------------
     * [3] 송신 직후 추가 안정화 지연
     * - 수신기에서 패킷 처리 완료 전 모드 전환 등 방지
     * ------------------------------------------------------------------------ */
    HAL_Delay(150);  // 전송 안정화 지연
}
/**
 * @brief 외부 설정 구조체 기반 E220 설정 적용
 */
void E220_ApplyConfig(E220 *_lora, const E220_Config *config) {
	_lora->Addr = config->addr;
	_lora->Channel = config->channel;
	_lora->Trans = config->transMode;
	_lora->Power = config->power;
	_lora->Baudrate = config->baudrate;
	_lora->FEC = config->fec;
	_lora->RSSI = config->rssi;
	_lora->WOR = config->wor;
	_lora->defaultMode = config->defaultMode;
}
/**
 * @brief 현재 LoRa 인스턴스 포인터 반환
 */
E220* GetLoRaInstance(void) {
	return &myLoRa;
}

void SetLoRaDestination(uint16_t addr) {
	    gDstInfo[0] = 0x00;
	    gDstInfo[1] = EEA_RTU_ID;
	    gDstInfo[2] = CHANNEL_ID;
	}
