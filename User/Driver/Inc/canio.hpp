//
// Author: Breezeee
// Date: 25-5-28
//

#ifndef CANIO_HPP
#define CANIO_HPP
#include <sys/_stdint.h>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"

// =============================== 调用库 ===============================

// =============================== 变量区 ===============================
typedef enum
{
    CAN_DMJ4310_M1_RECEIVE_ID = 0x011,
    CAN_DMJ4310_M2_RECEIVE_ID = 0x012,
    CAN_DMJ4310_M3_RECEIVE_ID = 0x013,
    CAN_DMJ4310_M4_RECEIVE_ID = 0x014,

    CAN_DMJ4310_M1_SEND_ID = 0x001,
    CAN_DMJ4310_M2_SEND_ID = 0x002,
    CAN_DMJ4310_M3_SEND_ID = 0x003,
    CAN_DMJ4310_M4_SEND_ID = 0x004,
}CAN1_ID;

typedef enum
{


}CAN2_ID;
// =============================== 函数声明 ===============================
void FDCAN_Init();
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

// =============================== 类声明 ===============================


#endif // CANIO_HPP
