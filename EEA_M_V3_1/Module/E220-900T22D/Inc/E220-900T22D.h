/*
 * E220_900T22D.h
 *
 * LoRa 모듈 드라이버 인터페이스 헤더 파일
 *      Author: PARK JANG HO
 */
#ifndef E220_900T22D_H
#define E220_900T22D_H

#include <E220_Config.h>
#include "stm32l4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>
#include "../../../Module/EEA_protocol/Inc/protocol.h"

/*----------------------[ E220 모드 정의 ]----------------------*/
#define SLEEP_MODE   0
#define CONFIG_MODE  1
#define TRANS_MODE   2

/*----------------------[ LoRa 모듈 구조체 정의 ]----------------------*/
///< 드라이버 구조체
typedef struct {
	UART_HandleTypeDef *pHuart;
	GPIO_TypeDef *M0Port;
	uint16_t M0Pin;
	GPIO_TypeDef *M1Port;
	uint16_t M1Pin;
	int Addr;
	int Channel;
	int Trans;
	int Baudrate;
	int Power;
	int current_mode;
	int FEC;
	int RSSI;
	int WOR;
	int defaultMode;
	ModuleType_t moduleType;
} E220;


/** @brief 전역에서 참조할 E220 인스턴스 선언 */
extern E220 myLoRa;

/** @brief LoRa 인스턴스 포인터 반환 */
E220* GetLoRaInstance(void);
/*----------------------[ 인터페이스 함수 선언 ]----------------------*/
void SetLoRaDestination(uint16_t addr);

/** @brief 기본값 구조체 생성 */
E220 NewLoRa(void);
/** @brief E220 초기화 (구조체 기반) */
void E220Init(E220 *_lora);
void InitLoRaModule(void);
void LoRa_AT_Setting(void);

///***********E220-SetMode(M0, M1 Setting)***********/
void E220LoRa_SetMode(E220 *_lora, int mode);
void E220LoRaSetSleepMode(E220 *_lora);
void E220LoRaSetTransmissionMode(E220 *_lora);
void E220LoRaSetConfigMode(E220 *_lora);
/***********E220-RFSettings(AT-COMMAND)***********/
void E220LoRa_SetAddr(E220 *_lora);
void E220LoRa_SetChannel(E220 *_lora);
void E220LoRa_SetTrans(E220 *_lora);
void E220LoRa_SetPower(E220 *_lora);

void E220LoRa_SetBaudrate(E220 *_lora);
void E220LoRa_SetFEC(E220 *_lora);
void E220LoRa_SetRSSI(E220 *_lora);
void E220LoRa_SetWOR(E220 *_lora);
bool E220_SetBaudrate(E220 *lora);
void E220_SetFEC(E220 *lora, uint8_t fec);
bool E220_SetRSSI(E220 *lora);
bool E220_SetWOR(E220 *lora);
bool E220_SaveSettings(E220 *lora);

bool LoRa_SetAddress_AT(uint16_t addr);
bool LoRa_SetChannel_AT(uint8_t ch);
bool LoRa_Save_AT(void);

int iE220LoRaMakePacket(const uint8_t *uchDstID, const uint8_t *uchData,
		int iDataLength, uint8_t *uchPaket);

/** 외부 설정 구조체 기반으로 E220 모듈 설정값을 적용 */
void E220_ApplyConfig(E220 *_lora, const E220_Config *config);

void LoRaSetMode(E220 *lora, LoRaMode_t mode);

/** @brief 데이터 송신 함수 */
void E220LoRaSendPacket(E220 *_lora, uint8_t *uchPacket, size_t iPacketLength);

#endif // E220_900T22D_H
