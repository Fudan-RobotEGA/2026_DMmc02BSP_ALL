//
// Author: Breezeee
// Date: 25-5-28
//

// =============================== 引入头文件 ===============================
#include "canio.hpp"
#include "usartio.hpp"
#include "motor.hpp"
// =============================== 宏定义区 ===============================

// =============================== 变量区 ===============================
extern FDCAN_HandleTypeDef hfdcan1;

// =============================== 函数实现 ===============================
/**
 * @brief  初始化FDCAN外设
 * @note   配置标准ID的接收过滤器，启用接收中断并启动FDCAN模块
 * @retval 无
 */
void FDCAN_Init()
{
    FDCAN_FilterTypeDef sFilterConfig;

    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_MASK;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0x000;
    sFilterConfig.FilterID2 = 0x000;

    if(HAL_FDCAN_ConfigFilter(&hfdcan1,&sFilterConfig)!=HAL_OK)
    {
        Error_Handler();
    }
    HAL_FDCAN_Start(&hfdcan1);
    HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

/**
 * @brief  FDCAN接收FIFO0中断回调函数
 * @param  hfdcan: 指向FDCAN句柄的指针
 * @param  RxFifo0ITs: FIFO0中断标志位（包括新消息、FIFO满、消息丢失等）
 * @note   处理FIFO0新接收消息的读取、过滤及处理，同时打印FIFO溢出和丢失信息
 * @retval 无
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0U)
    {
        FDCAN_RxHeaderTypeDef rx_header;
        uint8_t rx_data[8];
        if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
        {
            usart_printf("Error in FDCAN_GetRxMessage()\n");
            return;
        }
        if (hfdcan->Instance == FDCAN1)
        {
            uint32_t id = rx_header.Identifier;
            if (id == DMJ4310_M1.can_id_receive_)
            {
                DMJ4310_M1.UpdateInfoEncoder(rx_data);
            }
        }

        else if (hfdcan->Instance == FDCAN2)
        {
        }
        else if (hfdcan->Instance == FDCAN3)
        {
        }
    }

    // FIFO 满了提示
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_FULL) != 0U)
    {
        usart_printf("RX FIFO FULL\r\n");
    }
    // FIFO 溢出提示
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != 0U)
    {
        usart_printf("RX FIFO LOST\r\n");
    }
}