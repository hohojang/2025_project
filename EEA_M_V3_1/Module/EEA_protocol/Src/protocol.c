/**
 * @file protocol.c
 * @brief EEA 프로토콜 기반 패킷 송수신 및 처리 구현
 *      Author: PARK JANG HO
 */
#include "../../../Module/EEA_protocol/Inc/protocol.h"

#include <E220-900T22D.h>
#include <NNFQueue.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <Sensor.h>
#include "usart.h"
//------------------------------[ 외부 전역 변수 ]------------------------------//
extern volatile EEAPacket_t latestTxPacket;  ///< 마지막으로 전송된 EEA 패킷 정보 (재전송 및 상태 추적용)
extern volatile EEAPacket_t latestRxPacket;  ///< 마지막으로 수신된 유효한 EEA 패킷 구조체
extern CircularQueue rxQueue;                ///< 수신 패킷 처리를 위한 원형 큐 구조체

extern uint8_t rx_packet_buffer[EEA_MAX_PACKET_SIZE]; ///< 수신 패킷 임시 저장 버퍼 (파싱 전 단계)
extern uint8_t rx_index;                              ///< 현재 수신된 바이트 수 (패킷 파싱 인덱스)
volatile uint32_t rxPacketCount = 0;                  ///< 총 수신된 유효 패킷 수 (디버깅/통계용)
extern volatile uint8_t rx_byte;                      ///< UART 인터럽트로 수신된 1바이트 데이터
extern uint8_t gDstInfo[3];          ///< Fixed Mode 송신 시 사용되는 목적지 정보
extern UART_HandleTypeDef huart3;    ///< LoRa 통신에 사용되는 UART 핸들
uint32_t pirOffDelayMs = 10000;      ///< PIR 꺼짐 딜레이 시간 (ms 단위, 기본값 10)


/**
 * @brief 큐의 시작점에서 지정된 오프셋(offset) 위치의 요소 조회
 * @param q     CircularQueue 포인터
 * @param offset 시작 위치(iFront)로부터 떨어진 거리 (0 이상)
 * @param out   조회된 값을 저장할 포인터
 * @return true  성공적으로 값 조회
 * @return false 유효하지 않은 offset (큐 범위 밖)
 */
bool bPeekOffset(CircularQueue *q, int offset, element *out) {
    /* ------------------------------------------------------------------------
     * [1] Offset 유효성 검사
     * ------------------------------------------------------------------------ */
    // offset이 음수이거나, 큐에 들어있는 요소 개수보다 크거나 같으면 유효하지 않음
    if (offset < 0 || offset >= iGetQueueSize(q))
        return false;
    /* ------------------------------------------------------------------------
     * [2] 실제 배열 인덱스 계산 (원형 큐 고려)
     * ------------------------------------------------------------------------ */
    // offset만큼 이동한 실제 위치를 큐 용량으로 나눈 나머지로 계산
    int index = (q->iFront + offset) % q->iCapacity;
    /* ------------------------------------------------------------------------
     * [3] 해당 위치의 값을 out에 복사
     * ------------------------------------------------------------------------ */
    *out = q->array[index];
    /* ------------------------------------------------------------------------
     * [4] 정상 조회 성공
     * ------------------------------------------------------------------------ */
    return true;
}

/*==================================================================
 * @section 송신 처리
 *=================================================================*/
/**
 * @brief EEA 패킷 전송 (Fixed 모드 기반)
 * @param pkt 전송할 패킷 구조체 포인터
 * @return HAL_OK 항상 성공 반환
 *
 * @details 패킷 구성 → 직렬화 → E220 모듈을 통해 전송
 */
HAL_StatusTypeDef SendEEAPacket(const EEAPacket_t *pkt) {
	uint8_t data[64];   ///< 직렬화된 EEA 패킷
	int len = SerializeEEAPacket(data, pkt);

	uint8_t packet[64];  ///< Fixed 모드 송신 패킷
	int packetLen = iE220LoRaMakePacket(gDstInfo, data, len, packet);

	E220LoRaSendPacket(GetLoRaInstance(), packet, packetLen);

	memcpy((void*) &latestTxPacket, pkt, sizeof(EEAPacket_t));  ///< 직전에 전송한 패킷 저장

	return HAL_OK;
}
/**
 * @brief RTU로부터 수신된 PIR 꺼짐 시간 설정 요청을 처리
 * @param payload 수신된 데이터 (payload[0]에 설정 시간 값이 초 단위로 들어옴)
 * @param length payload 배열의 길이 (최소 1이어야 유효)
 * @details
 * - payload[0] 값은 초 단위의 PIR OFF 지연시간 (예: 30 → 30초)
 * - 내부 변수 pirOffDelayMs는 ms 단위로 저장되므로 *1000 처리
 * - 필요 시, 현재 PIR 상태를 기준으로 타이머 재설정 등의 추가 로직 구현 가능
 */
void HandlePirOffDelayRequest(const uint8_t* payload, uint8_t length) {
    if (length >= 1) {
    	pirHoldDuration  = payload[0] * 1000;  // 1바이트 값(초)을 ms 단위로 변환하여 저장
        // 예: payload[0] = 30 → 30초 지연시간 설정 (30000ms)
    }
}
//-----------------------------[ 수신 패킷 처리 ]-------------------------------------//
/**
 * @brief LoRa 수신 큐에서 패킷 추출 및 처리
 * @details
 * - Fixed Mode(3바이트 헤더) 기반 패킷 파싱
 * - 유효한 EEA 패킷을 역직렬화 후 명령어 처리 함수로 전달
 */
void ProcessLoRaReception(void) {
   static bool inPacket = false;
   static uint8_t tempBuf[EEA_MAX_PACKET_SIZE];
   static uint16_t tempIndex = 0;
   static uint32_t stxStartTime = 0;

   while (!bIsQueueEmpty(&rxQueue)) {
      element byte;
      bDequeue(&rxQueue, &byte);

      // [1] 타임아웃 검사
      if (inPacket && (HAL_GetTick() - stxStartTime > 10)) {
         inPacket = false;
         tempIndex = 0;
         continue;
      }

      // [2] STX 수신 대기
      if (!inPacket) {
         if (byte == EEA_STX) {
            inPacket = true;
            tempBuf[0] = byte;
            tempIndex = 1;
            stxStartTime = HAL_GetTick();
         }
         continue;
      }

      // [3] 수신 중: 바이트 누적
      tempBuf[tempIndex++] = byte;

      // [4] 버퍼 초과 방지
      if (tempIndex >= EEA_MAX_PACKET_SIZE) {
         inPacket = false;
         tempIndex = 0;
         continue;
      }

      // [5] 최소 길이 도달 후 LEN 추출
      if (tempIndex >= 6) {
         uint8_t payloadLen = tempBuf[5];
         if (payloadLen > EEA_MAX_DATA_SIZE) {
            // 잘못된 길이 → 초기화
            inPacket = false;
            tempIndex = 0;
            continue;
         }

         uint8_t totalLen = 6 + payloadLen + 2;

         // [6] ETX 수신 && 길이 일치 시 패킷 처리
         if (byte == EEA_ETX && tempIndex == totalLen) {
            inPacket = false;

            uint8_t calcChecksum = CalculateChecksum(tempBuf,
                  6 + payloadLen);
            uint8_t recvChecksum = tempBuf[6 + payloadLen];

            if (recvChecksum == calcChecksum) {
               EEAPacket_t pkt = { .stx = tempBuf[0], .srcId = tempBuf[1],
                     .dstId = tempBuf[2], .blockId = tempBuf[3],
                     .command = tempBuf[4], .length = payloadLen,
                     .checksum = recvChecksum, .etx = tempBuf[6
                           + payloadLen + 1] };
               memcpy(pkt.data, &tempBuf[6], payloadLen);

               // 유효 ID 확인
               if ((pkt.dstId == EEA_M_ID || pkt.dstId == 0xFF)
                     && pkt.srcId == EEA_RTU_ID) {

                  memcpy((void*) (volatile void*) &latestRxPacket, &pkt,
                        sizeof(EEAPacket_t));
                  rxPacketCount++;
                  SetLoRaDestination(pkt.srcId);

                  // 명령 처리
                  switch (pkt.command) {
                  case CMD_LED_CTRL_RES:
                     LedControlCommand(pkt.data, pkt.length);
                     break;

                  case CMD_PIR_OFF_DELAY_REQ:
                     PirDelayCommand(pkt.data, pkt.length, pkt.srcId);
                     break;

                  case CMD_LED_FAULT_RES:
                  case CMD_PIR_OFF_DELAY_RES:
                     break;

                  default:
                     break;
                  }
               }
            }

            // regardless of result
            tempIndex = 0;
            continue;
         }

         // [7] ETX가 왔지만 길이 부족 → 폐기
         if (byte == EEA_ETX && tempIndex < totalLen) {
            inPacket = false;
            tempIndex = 0;
            continue;
         }
      }
   }
}
/**
 * @brief EEAPacket_t 구조체를 직렬화하여 바이트 배열로 변환
 * @param dest 변환된 바이트 배열을 저장할 버퍼 포인터
 * @param pkt 직렬화할 EEA 패킷 구조체 포인터
 * @return 직렬화된 전체 바이트 길이
 */
int SerializeEEAPacket(uint8_t *dest, const EEAPacket_t *pkt) {
	int index = 0;
	dest[index++] = pkt->stx;       ///< 패킷 시작 바이트 (0x02)
	dest[index++] = pkt->srcId;     ///< 송신자 ID
	dest[index++] = pkt->dstId;     ///< 수신자 ID
	dest[index++] = pkt->blockId;   ///< 블록 ID (센서 등)
	dest[index++] = pkt->command;   ///< 명령어
	dest[index++] = pkt->length;    ///< 데이터 길이

	///< 데이터 필드 복사
	for (int i = 0; i < pkt->length; i++) {
		dest[index++] = pkt->data[i]; ///< 각 데이터 바이트 복사
	}

	dest[index++] = pkt->checksum;  ///< 체크섬
	dest[index++] = pkt->etx;       ///< 패킷 종료 바이트 (0x03)

	return index; ///< 전체 직렬화된 바이트 수 반환
}
