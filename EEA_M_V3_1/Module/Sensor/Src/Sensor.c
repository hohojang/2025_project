/**
 * @file Sensor.c
 * @brief 센서 통합 제어 및 LoRa 패킷 전송 처리
 *      Author: PARK JANG HO
 */
#include <stdio.h>
#include <string.h>
#include "Flash.h"
#include <E220-900T22D.h>
#include <HC-SR501PirSensor.h>
#include <LedSensor.h>
#include <Sensor.h>
#include <E220_Config.h>
#include <SZH-SSBH-011CdsSensor.h>
#include "../../../Module/EEA_protocol/Inc/protocol.h"

#define HEADER_SIZE 6  ///< STX~LEN까지
uint8_t rx_packet_buffer[EEA_MAX_PACKET_SIZE] = { 0 };
uint8_t rx_index = 0;
/*------------------[ PIR 상태 유지 관련 타이머 ]------------------*/
uint32_t pirHoldDuration = PIR_HOLD_TIME_MS; ///< 사람 감지 후 LED 유지 시간
uint32_t lastPirDetectionTime = 0;           ///< 마지막 PIR 감지 시간 (LED OFF 조건 판별)

/*------------------[ M 상시 제어 중단 조건 관련 ]------------------*/
bool manualLedOverride = false;  ///< 외부 제어가 활성화되어 있는지 여부

/*------------------[ PIR 상태 변화 감지 처리 ]------------------*/
uint8_t currentPirStatus = PIR_NOT_DETECTED;  ///< 현재까지 안정적으로 감지된 PIR 상태
uint8_t lastPirStatus = PIR_NOT_DETECTED;     ///< 마지막으로 전송한 PIR 상태
uint32_t pirChangeStartTime = 0;              ///< PIR 상태 변화 발생 시 타이머 시작 시간
bool pirChangePending = false;                ///< 상태 변화가 감지되었고 확정 대기 중인지 여부

/*------------------[ LED 상태 전송 추적 ]------------------*/
uint8_t lastReportedLedStatus = LED_OFF;      ///< 마지막으로 전송한 LED 상태

/*------------------[ CDS 오류 감지 상태 추적 ]------------------*/
uint8_t lastCdsError = CDS_NO_ERROR;        ///< 마지막으로 확정된 CDS 오류 상태
uint32_t cdsErrorStartTime = 0;             ///< CDS 오류 의심 시작 시점
bool cdsErrorPending = false;               ///< CDS 오류 의심 중인지 여부

/*-----------------------------[ 외부 핸들 선언 ]-----------------------------*/
extern UART_HandleTypeDef huart1;
extern E220 myLoRa;

uint8_t gDstInfo[3] = { DST_HIGH_ID, EEA_RTU_ID, CHANNEL_ID }; // dstHigh, dstLow, channel

/**
 * @brief PIR, CDS, LED 센서를 모두 초기화
 * - 센서별 내부 GPIO, 변수, ADC 설정 등을 각각 초기화
 * - 센서 통합 초기화 진입점으로 사용
 */
void InitAllSensors(void) {
    /* ------------------------------------------------------------------------
     * [1] CDS 조도 센서 초기화
     * ------------------------------------------------------------------------ */
    InitCdsSensor();
    /* ------------------------------------------------------------------------
     * [2] PIR 인체 감지 센서 초기화
     * ------------------------------------------------------------------------ */
    InitPirSensor();
    /* ------------------------------------------------------------------------
     * [3] 전등(LED) 제어용 센서/제어핀 초기화
     * ------------------------------------------------------------------------ */
    InitLedSensor();
}
/**
 * @brief LED 고장 상태를 반환 (CDS 센서 기반)
 * @details
 * - LED가 꺼진 경우는 고장 판단 대상이 아님
 * - CDS 센서 기준값 비교 결과를 기반으로 고장 여부 판별
 */
uint8_t GetLedFaultStatus(void) {
    /* ------------------------------------------------------------------------
     * [1] LED가 ON 상태일 때만 CDS 기반 고장 판단 수행
     * ------------------------------------------------------------------------ */
    if (GetLedState() == LED_ON &&
        DetectLedFaultByCds() == CDS_ERROR) {
        return 1;  ///< 고장 상태
    }
    /* ------------------------------------------------------------------------
     * [2] 그 외에는 모두 정상 처리
     * - LED OFF 중이거나 CDS 감지값이 정상일 경우
     * ------------------------------------------------------------------------ */
    return 0;      ///< 정상 상태
}
/**
 * @brief PIR 센서 감지 시 호출되는 처리 함수
 * @param now 현재 시스템 시간 (ms 단위)
 * @details
 * - 감지 시각을 기록하고, 이전 상태와 비교해 감지 상태 보고
 * - LED를 ON 상태로 설정하고 RTU에 상태 보고
 */
void PirDetected(uint32_t now) {
    /* ------------------------------------------------------------------------
     * [1] 마지막 감지 시각 갱신
     * ------------------------------------------------------------------------ */
    lastPirDetectionTime = now;

    /* ------------------------------------------------------------------------
     * [2] 이전 상태가 미감지였을 경우에만 PIR 상태 보고
     * - 동일 상태 반복 보고 방지
     * ------------------------------------------------------------------------ */
    if (lastPirStatus != PIR_DETECTED) {
        SendPirStatusPacket(PIR_DETECTED);  ///< RTU에 PIR 감지 상태 보고
        lastPirStatus = PIR_DETECTED;       ///< 내부 상태 갱신
    }

    /* ------------------------------------------------------------------------
     * [3] LED ON 및 상태 보고
     * - 이미 켜져 있지 않은 경우에만 동작
     * ------------------------------------------------------------------------ */
    if (GetLedState() != LED_ON) {
        SetLedState(LED_ON);
        SendLedStateReport(LED_ON);
    }
}
/**
 * @brief PIR 미감지 상태가 일정 시간 유지되었을 때 LED OFF 처리
 * @param now 현재 시스템 시간 (ms 단위)
 * @details
 * - 마지막 PIR 감지 이후 지정된 유지 시간이 경과하면 LED를 OFF함
 * - 이미 꺼져 있는 경우, 다시 감지된 경우, 유지 시간이 미도달 시 무시
 */
void PirHoldTimeout(uint32_t now) {
    /* ------------------------------------------------------------------------
     * [1] 현재 LED가 이미 꺼져 있으면 처리 불필요
     * ------------------------------------------------------------------------ */
    if (GetLedState() != LED_ON)
        return;
    /* ------------------------------------------------------------------------
     * [2] PIR 유지 시간 미도달 시 대기
     * - 감지 이후 일정 시간(pirHoldDuration) 유지 조건을 만족하지 않으면 리턴
     * ------------------------------------------------------------------------ */
    if ((now - lastPirDetectionTime) < pirHoldDuration)
        return;
    /* ------------------------------------------------------------------------
     * [3] 아직 PIR이 감지된 상태라면 종료
     * - PIR 미감지 상태에서만 OFF 처리 가능
     * ------------------------------------------------------------------------ */
    if (IsPirstate() == PIR_DETECTED)
        return;

    if (IsPirstate() == PIR_NOT_DETECTED)
    SetLedState(LED_OFF);                 ///< 실제 LED OFF
    SendLedStateReport(LED_OFF);          ///< RTU에 전등 OFF 상태 보고
    lastPirStatus = PIR_NOT_DETECTED;     ///< 내부 PIR 상태 갱신 (패킷은 전송 안 함)
}

/**
 * @brief CDS 센서를 기반으로 전등 점등 실패(고장)를 판단하고 RTU에 보고
 * @param cdsFault 현재 CDS 센서에서 감지된 오류 상태
 *        - true  : LED가 ON 상태임에도 조도가 기준 이하 (고장 의심)
 *        - false : 조도 정상
 * @param now 현재 시스템 시간 (ms 단위, HAL_GetTick 기반)
 * @details
 * - 조도가 기준 이하일 경우 일정 시간 동안 유지되면 전등 고장으로 간주
 * - 중복 전송 방지를 위해 이전 전송 여부 상태(`lastCdsError`)와 비교
 * - 고장 발생 시에는 전송하고, 정상 복귀 시에는 상태만 리셋 (보고는 선택사항)
 */
void CheckLightFail(bool cdsFault, uint32_t now) {
    /* ------------------------------------------------------------------------
     * [0] 플래시 메모리에서 이전 오류 상태 로드 (시작 시 1회 실행)
     * ------------------------------------------------------------------------ */
    static bool isInitialized = false;
    if (!isInitialized) {
        lastCdsError = (ReadFaultStatus() == CDS_ERROR_FLASH_ID) ? CDS_ERROR : CDS_NO_ERROR;
        isInitialized = true;
    }

    /* ------------------------------------------------------------------------
     * [1] CDS 이상 상태 감지 (LED는 ON인데 어두운 경우)
     * ------------------------------------------------------------------------ */
    if (cdsFault) {
        if (!cdsErrorPending) {
            cdsErrorPending = true;
            cdsErrorStartTime = now;
        }
        else if ((now - cdsErrorStartTime) >= CDS_RETRY_TIMEOUT_MS &&
                 lastCdsError == CDS_NO_ERROR)
        {
            SendLightFail(true);
            lastCdsError = CDS_ERROR;

            // [추가] 오류 상태 플래시 메모리 저장
            SaveFaultStatus(CDS_ERROR_FLASH_ID); // 0xA5A5A5A5
        }
    }
    /* ------------------------------------------------------------------------
     * [2] CDS 정상 상태 복귀 처리
     * ------------------------------------------------------------------------ */
    else if (lastCdsError == CDS_ERROR) {
        if ((now - cdsErrorStartTime) >= CDS_ERROR_HOLD_TIME_MS) {
            SendLightFail(false);
            cdsErrorPending = false;
            lastCdsError = CDS_NO_ERROR;

            // [추가] 정상 상태 플래시 메모리 저장
            SaveFaultStatus(CDS_NO_ERROR_FLASH_ID); // 0x00000000
        }
    }
}

/**
 * @brief 수신 패킷의 체크섬 계산 (RTU 방식: STX부터 Data까지 덧셈)
 * @param raw 바이트 배열 포인터
 * @param length 체크섬 계산 대상 길이 (STX ~ 마지막 Data까지)
 * @retval 계산된 체크섬 (8비트 덧셈 누적 결과)
 */
uint8_t CalculateChecksum(const uint8_t *raw, uint8_t length) {
    uint8_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += raw[i];
    }
    return sum;
}
/**
 * @brief PIR / CDS 상태에 따라 자동 전등 제어 및 고장 점검 처리
 * @details
 * - CDS 센서를 이용한 LED 고장 감지
 * - 일정 시간 이상 수동 제어가 없으면 자동 제어로 복귀
 * - PIR 감지에 따라 LED 자동 제어 수행
 */
void ProcessAllSensors(void) {
    /* ------------------------------------------------------------------------
     * [1] CDS 센서 기반 LED 이상 상태 점검
     * - CDS 값이 정상 동작 범위를 벗어나면 CheckLightFail()에서 패킷 전송
     * ------------------------------------------------------------------------ */
    uint8_t cds  = DetectLedFaultByCds();     ///< CDS 값 측정 (LED 점등 상태 기반)
    uint32_t now = HAL_GetTick();            ///< 현재 시간 획득
    CheckLightFail(cds, now);                ///< CDS 이상 여부 판단 및 보고

    /* ------------------------------------------------------------------------
     * [2] 수동 제어 중이면 자동 제어 중단
     * ------------------------------------------------------------------------ */
    if (manualLedOverride)
        return;
    /* ------------------------------------------------------------------------
     * [3] PIR 센서 상태에 따라 LED 자동 제어 수행
     * - 감지 시: LED ON + PIR 상태 보고
     * - 미감지 시: 일정 시간 후 LED OFF
     * ------------------------------------------------------------------------ */
    if (IsPirstate() == PIR_DETECTED) {
        PirDetected(now);          ///< PIR 감지 → LED ON + 상태 보고
    } else {
        PirHoldTimeout(now);       ///< PIR 미감지 지속 시 → LED OFF 처리
    }
}
/**
 * @brief 공용 EEA 데이터 패킷 전송 함수
 * @param dstId 수신 대상 디바이스 ID (예: RTU_ID)
 * @param command 명령어 코드
 * @param payload 전송할 데이터 (NULL 가능, 길이 0인 경우 사용 안 함)
 * @param length payload의 길이 (최대 32 바이트)
 */
void SendEEADataPacket(uint8_t dstId, uint8_t command, uint8_t *payload, uint8_t length) {
    SetLoRaDestination(dstId); ///< [1] LoRa 대상 설정

    EEAPacket_t pkt = {
        .stx     = EEA_STX,
        .srcId   = EEA_M_ID,
        .dstId   = dstId,
        .blockId = BLOCK_ID_SENSOR,
        .command = command,
        .length  = length,
        .etx     = EEA_ETX
    };

    if (length > 0 && payload != NULL) {
        memcpy(pkt.data, payload, length);
    }

    // [1] STX~DATA까지를 기준으로 Checksum 계산
    uint8_t rawBuf[HEADER_SIZE + pkt.length];
    rawBuf[0] = pkt.stx;
    rawBuf[1] = pkt.srcId;
    rawBuf[2] = pkt.dstId;
    rawBuf[3] = pkt.blockId;
    rawBuf[4] = pkt.command;
    rawBuf[5] = pkt.length;
    for (int i = 0; i < pkt.length; i++) {
        rawBuf[6 + i] = pkt.data[i];
    }

    pkt.checksum = CalculateChecksum(rawBuf, HEADER_SIZE + pkt.length);

    // [2] 마지막 전송 패킷 기록
    memcpy((void *)(volatile void *)&latestTxPacket, &pkt, sizeof(EEAPacket_t));

    // [3] 직렬화 → 송신 패킷 구성 → 전송
    uint8_t data[16];
    int len = SerializeEEAPacket(data, &pkt); ///< [2] EEA 패킷 직렬화

    uint8_t packet[64];
    int packetLen = iE220LoRaMakePacket(gDstInfo, data, len, packet); ///< [3] LoRa 포맷화
    E220LoRaSendPacket(&myLoRa, packet, packetLen); ///< [4] 전송
}
/**
 * @brief 주기적인 LED 상태 보고 패킷 전송 (CMD_LED_CTRL_REQ = 0x02)
 * @param ledStatus 현재 전등 상태 (LED_ON 또는 LED_OFF)
 * @note 항상 전송 (중복 방지 제거)
 */
void SendLedStateReport(uint8_t ledStatus) {
    uint8_t payload = ledStatus;
    SendEEADataPacket(EEA_RTU_ID, CMD_LED_CTRL_REQ, &payload, 1);
    lastReportedLedStatus = ledStatus;
}

/**
 * @brief PIR 감지 상태 보고 패킷 전송 (CMD_PIR_MOVE_REQ = 0x03)
 * @note 상태 변화가 있을 때만 보고
 */
void SendPirStatusPacket(bool detected) {
    if (!detected) return;

    uint8_t payload = 0x01;
    SendEEADataPacket(EEA_RTU_ID, CMD_PIR_MOVE_REQ, &payload, 1);
    lastPirStatus = PIR_DETECTED;
}
/**
 * @brief CDS 센서 기반 전등 고장 또는 복구 상태를 RTU에 보고
 * @param fault true = 고장 발생(0x01), false = 고장 복구(0x00)
 */
void SendLightFail(bool fault) {
    uint8_t payload = fault ? 0x01 : 0x00;
    SendEEADataPacket(EEA_RTU_ID, CMD_LED_FAULT_REQ, &payload, 1);
}
/**
 * @brief PIR OFF 지연시간 설정에 대한 응답 전송 (CMD_PIR_OFF_DELAY_RES)
 */
void SendPirOffTimeResponse(uint8_t dstId) {
    uint8_t ack = 0x01;
    SendEEADataPacket(dstId, CMD_PIR_OFF_DELAY_RES, &ack, 1);
}

/**
 * @brief PIR 감지 해제 후 LED 소등까지의 지연 시간 설정
 * @param payload 수신 데이터 (payload[0] = MSB, payload[1] = LSB)
 * @param length  payload 길이 (최소 2 이상)
 * @param srcId   요청 송신자 ID (응답 전송 시 사용)
 */
void PirDelayCommand(const uint8_t *payload, uint8_t length, uint8_t srcId) {
	if (length >= 2) { ///< payload가 2바이트 이상인지 확인
		///< payload[0]: 상위 바이트, payload[1]: 하위 바이트
		///< 두개의 8비트 데이터를 하나의 16비트 값으로 합침
		pirHoldDuration = (payload[0] << 8) | payload[1];
		printf("[CMD] PIR OFF 시간 설정: %lu ms\n", pirHoldDuration);
		SendPirOffTimeResponse(srcId); ///< 요청 송신자 주소로 응답 패킷 전송
	}
}
/**
 * @brief 수동 전등 제어 명령을 처리
 * @param payload 제어 명령 데이터
 *        - payload[0] = 0xFF: 수동 전등 ON, 자동 제어 중단
 *        - payload[0] = 0x00: 수동 전등 OFF, 자동 제어 복귀
 * @param length  payload 길이
 */
void LedControlCommand(const uint8_t *payload, uint8_t length) {
    if (length < 1)
        return;

    uint8_t cmd = payload[0];
    bool ledChanged = false;

    /* 1. CS 브로드캐스트 명령 처리 */
    if (cmd == 0xFF) {
        // 수동 전등 ON → 자동 제어 중단
        if (!GetLedState()) {
            TurnOnLed();
            ledChanged = true;
        }
        manualLedOverride = true;
    }
    else if (cmd == 0x00) {
        // 수동 전등 OFF → 자동 제어 복귀
        if (GetLedState()) {
            TurnOffLed();
            ledChanged = true;
        }
        manualLedOverride = false;
    }
    /* 2. RTU 전등 제어 */
    else if (cmd == 0x01) {
        if (!GetLedState()) {
            TurnOnLed();
            ledChanged = true;
        }
    }
    /* 3. LED 상태 변화 시 상태 보고 */
    if (ledChanged) {
    	SendLedStateReport(GetLedState() ? 0x01 : 0x00);

    }
}
