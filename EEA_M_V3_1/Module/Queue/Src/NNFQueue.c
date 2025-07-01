/*
 * Queue.c
 *
 *  Created on: Nov 8, 2023
 *      Author: rnqhs
 */
#include "NNFQueue.h"
#include <stddef.h>
/**
 * @brief 큐 초기화
 * @param hQueue 큐 구조체 포인터
 * @param array 데이터 저장에 사용할 배열 포인터
 * @param _QueueSize 배열 크기 (capacity)
 * @retval true 초기화 성공
 * @retval false 배열이 NULL인 경우
 */
bool bQueueinit(CircularQueue *hQueue, element *array, uint32_t _QueueSize) {
	if (array == NULL)
		return false;               ///< 배열이 없으면 큐를 만들 수 없으니 실패 반환
	hQueue->array = array;          ///< 큐가 사용할 저장 공간 정함(array)
	hQueue->iFront = 0;             ///< 큐의 맨 앞 front 0으로 초기화
	hQueue->iRear = 0;              ///< 큐의 맨 뒤 rear 0으로 초기화
	hQueue->iCapacity = _QueueSize; ///< 큐 최대 크기 설정
	hQueue->iSize = 0;              ///< 현재 요소 수 0으로 설정(큐가 비어 있다고 표시)
	return true;
}
/**
 * @brief 큐가 비어있는지 확인
 * @param hQueue 큐 구조체 포인터
 * @retval true 비어 있음
 * @retval false 데이터 존재
 */
///< bool 타입의 bIsQueueEmpty 함수의 인자로 원형 큐 구조체를 가진 큐 포인터
bool bIsQueueEmpty(CircularQueue *hQueue) {
	  ///< 큐의 Rear 와 Front 의 위치가 같을 시 큐가 비어있다고 판단
	return hQueue->iRear == hQueue->iFront;
}
/**
 * @brief 큐가 가득 찼는지 확인
 * @param hQueue 큐 구조체 포인터
 * @retval true 가득 참
 * @retval false 공간 있음
 */
///< bool 타입의 bIsQueueFull 함수의 인자로 원형 큐 구조체를 가진 큐 포인터
bool bIsQueueFull(CircularQueue *hQueue) {
	///< 다음 데이터가 들어올 위치를 계산 후
	///<전체 배열의 크기로 나눈 크기가 front와 같으면 큐가 가득 찼다고 판단
	return ((hQueue->iRear + 1) % hQueue->iCapacity) == hQueue->iFront;
}
/**
 * @brief 큐에 데이터 삽입
 * @param hQueue 큐 구조체 포인터
 * @param _data 삽입할 데이터
 * @retval true 삽입 성공
 * @retval false 큐가 가득 참
 */
///< bool 타입의 bEnqueue 함수의 인자로 원형 큐 구조체를 가진 큐 포인터와 큐에 넣을 데이터
bool bEnqueue(CircularQueue *hQueue, uint8_t data) {
    ///< 1. 큐가 가득 찼는지 먼저 확인
    if (bIsQueueFull(hQueue))
        return false;  // 큐가 가득 차 있으면 데이터를 넣지 않고 false(실패)를 반환

    ///< 2. rear가 가리키는 배열 위치에 새로운 데이터를 저장
    hQueue->array[hQueue->iRear] = data;

    ///< 3. rear를 다음 위치로 이동
    ///<  (현재 rear에 1을 더한 뒤, 전체 배열 크기로 나눈 나머지 값을 rear에 저장)
    ///<  이렇게 하면 rear가 배열의 끝에 도달했을 때 다시 0번 인덱스로 돌아옴
    hQueue->iRear = (hQueue->iRear + 1) % hQueue->iCapacity;

    ///< 4. 데이터 삽입이 성공했으니 true(성공)를 반환
    return true;
}

/**
 * @brief 큐에서 데이터 추출
 * @param hQueue 큐 구조체 포인터
 * @param Data 추출한 데이터를 저장할 포인터
 * @retval true 추출 성공
 * @retval false 큐가 비어 있음
 */
///< bool 타입의 bDequeue 함수의 인자로 원형 큐 구조체를 가진 큐 포인터와 큐에 넣을 데이터
bool bDequeue(CircularQueue *hQueue, element *Data) {
	///< 큐가 비어있는지 확인
	if (bIsQueueEmpty(hQueue))
		return false; ///< 큐가 비어 있으면 추출 불가
	*Data = hQueue->array[hQueue->iFront];   ///< front 위치의 데이터 추출
	///< front를 큐의 다음 위치로 이동시키는데, 큐의 끝이면 처음으로 돌아감
	hQueue->iFront = (hQueue->iFront + 1) % hQueue->iCapacity;
	return true;
}
/**
 * @brief 이 함수는 큐에서 데이터를 꺼내지 않고,
 * 맨 앞에 있는 데이터가 무엇인지 확인(조회)만 할 때 사용
 * @param hQueue 큐 구조체 포인터
 * @param data 조회한 데이터를 저장할 포인터
 * @retval true 조회 성공
 * @retval false 큐가 비어 있음
 */
///< bool 타입의 bPeek 함수의 인자로 원형 큐 구조체를 가진 큐 포인터와 큐에 넣을 데이터
bool bPeek(CircularQueue *hQueue, element *data) {
	if (bIsQueueEmpty(hQueue))
		return false; ///< 비어 있으면 조회 불가
	///< 큐의 맨 앞(front) 위치에 있는 데이터를 꺼내서 함수 인자로 받은 data(포인터)에 복사
	*data = hQueue->array[hQueue->iFront];
	return true;
}
/**
 * @brief 큐에 저장된 요소 개수 반환
 * @param hQueue 큐 구조체 포인터
 * @param 큐에 현재 몇 바이트가 들어있는지 확인
 * @return 현재 큐의 요소 개수
 */
///< int 타입의 iGetQueueSize 함수의 인자로 원형 큐 구조체를 가진 큐 포인터
int iGetQueueSize(CircularQueue *hQueue) {
	///<  front가 rear보다 뒤에 있을 때
	if (hQueue->iFront > hQueue->iRear) {

		///< (전체 크기 - front 위치) + rear 위치로 현재 큐에 있는 데이터 개수를 계산
		hQueue->iSize = hQueue->iCapacity - hQueue->iFront + hQueue->iRear;
	} else {
		hQueue->iSize = hQueue->iRear - hQueue->iFront;
	}
	return hQueue->iSize;
}
/**
 * @brief 특정 길이 삽입 시 overflow 여부 확인
 * @param hQueue 큐 구조체 포인터
 * @param _uiLen 삽입 시도 길이
 * @retval true overflow 발생
 * @retval false 정상 삽입 가능
 */
bool bIsOverFlow(CircularQueue *hQueue, uint32_t _uiLen) {
	return (hQueue->iFront + _uiLen > hQueue->iCapacity); ///< 단순 계산 기준으로 overflow 판단
}
/**
 * @brief front 기준으로 남은 버퍼 공간 계산
 * @param hQueue 큐 구조체 포인터
 * @return front부터 끝까지의 남은 공간
 */
int iGetSubCapFront(CircularQueue *hQueue) {
	return hQueue->iCapacity - hQueue->iFront; ///< 현재 front부터 끝까지의 공간 반환
}
/**
 * @brief overflow 발생 시 인덱스를 순환 처리하여 반환
 * @param hQueue 큐 구조체 포인터
 * @param _uiLen 길이
 * @return front + len의 순환된 인덱스
 */
int igetOverFlowCount(CircularQueue *hQueue, uint32_t _uiLen) {
	return (hQueue->iFront + _uiLen) % hQueue->iCapacity; ///< wrap-around된 인덱스 반환
}
/**
 * @brief 큐의 front를 지정한 길이만큼 이동시킴 (삭제 효과)
 * @note 큐가 비어 있지 않고, 이동 길이가 현재 size 이하일 것
 * @param hQueue 큐 구조체 포인터
 * @param _uiLen 이동할 길이
 */
void vMoveFront(CircularQueue *hQueue, uint32_t _uiLen) {
	hQueue->iFront = (hQueue->iFront + _uiLen) % hQueue->iCapacity; ///< front 위치를 순환 이동
}
/**
 * @brief front 또는 rear를 서로 맞추는 동기화 함수
 * @details front > rear → rear = front, front < rear → front = rear
 * @param hQueue 큐 구조체 포인터
 * @retval true 동기화 성공
 * @retval false 큐가 비어 있음
 */
uint8_t uchmoveFrontToRear(CircularQueue *hQueue) {
	if (bIsQueueEmpty(hQueue))
		return false; ///< 비어 있으면 동기화 불필요
	if (hQueue->iFront > hQueue->iRear) {
		hQueue->iRear = hQueue->iFront % hQueue->iCapacity; ///< front가 앞서 있으면 rear를 맞춤
	} else if (hQueue->iFront < hQueue->iRear) {
		hQueue->iFront = hQueue->iRear % hQueue->iCapacity; ///< rear가 앞서 있으면 front를 맞춤
	}
	return true;
}
