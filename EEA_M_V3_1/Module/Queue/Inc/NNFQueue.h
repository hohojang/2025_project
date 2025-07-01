/*
 * Queue.h
 *
 *  Created on: Nov 8, 2023
 *      Author: rnqhs
 */

#ifndef _QUEUE_H_QUEUE_H_
#define _QUEUE_H_QUEUE_H_

#include "stdint.h"
#include "stdbool.h"

//---------Define---------//

//---------typedef---------//
typedef uint8_t element; ///< element를 uint8_t 타입으로 지정

typedef struct {
	element *array;   ///< 큐의 데이터를 저장할 배열의 포인터
	int iFront;       ///< 데이터를 꺼낼 위치를 가리키는 인덱스
	int iRear;        ///< 데이터를 삽입할 위치를 가리키는 인덱스
	int iSize;        ///< 현재 큐에 저장된 데이터 개수
	int iCapacity;    ///< 큐가 저장할 수 있는 최대 용량
} CircularQueue;

bool bQueueinit(CircularQueue *hQueue, element *array, uint32_t _QueueSize);
bool bIsQueueEmpty(CircularQueue *hQueue);
bool bIsQueueFull(CircularQueue *hQueue);
bool bEnqueue(CircularQueue *hQueue, element _data);
bool bDequeue(CircularQueue *hQueue, element *Data);
bool bPeek(CircularQueue *hQueue, element *data);
int iGetQueueSize(CircularQueue *hQueue);
bool bIsOverFlow(CircularQueue *hQueue, uint32_t _uiLen);
int iGetSubCapFront(CircularQueue *hQueue);
int igetOverFlowCount(CircularQueue *hQueue, uint32_t _uiLen);
void vMoveFront(CircularQueue *hQueue, uint32_t _uiLen);
uint8_t uchmoveFrontToRear(CircularQueue *hQueue);
#endif /* QUEUE_H_QUEUE_H_ */
