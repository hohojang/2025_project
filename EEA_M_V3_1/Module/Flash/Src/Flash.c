#include "Flash.h"
#include <stdint.h>

// 페이지 주소 계산 (HAL의 FLASH_PAGE_SIZE 사용)
static uint32_t GetPage(uint32_t Addr) {
    return ((Addr - FLASH_BASE) / FLASH_PAGE_SIZE);
}

HAL_StatusTypeDef SaveFaultStatus(uint32_t status) {
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Page = GetPage(FLASH_USER_START_ADDR),
        .NbPages = 1,
        .Banks = FLASH_BANK_1  // 단일 뱅크 설정
    };

    uint32_t PageError;
    if (HAL_FLASHEx_Erase(&erase, &PageError) != HAL_OK) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    // 64비트 데이터 무결성 저장
    uint64_t data = (uint64_t)status | ((uint64_t)(~status) << 32);

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD,
                          FLASH_USER_START_ADDR,
                          data) != HAL_OK) {
        HAL_FLASH_Lock();
        return HAL_ERROR;
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

uint32_t ReadFaultStatus(void) {
    uint64_t data = *(volatile uint64_t*)FLASH_USER_START_ADDR;
    uint32_t status = data & 0xFFFFFFFF;
    uint32_t complement = (data >> 32) & 0xFFFFFFFF;
    return (status == (uint32_t)~complement) ? status : 0xFFFFFFFF;
}
