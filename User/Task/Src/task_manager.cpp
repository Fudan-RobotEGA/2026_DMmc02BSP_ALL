//
// Author: Breezeee
// Date: 25-5-28
//

// =============================== 引入头文件 ===============================
#include "task_manager.hpp"
#include "canio.hpp"
#include "led.hpp"
#include "cmsis_os.h"
#include "motor.hpp"
#include "usartio.hpp"
#include "debug.hpp"
// =============================== 宏定义区 ===============================

// =============================== 变量区 ===============================
extern FDCAN_HandleTypeDef hfdcan1;
// =============================== 函数实现 ===============================
/**
 * @brief  创建系统任务
 * @note   使用 FreeRTOS 的 xTaskCreate 注册所有任务入口。
 *         可在此处添加更多应用任务，实现系统模块化调度。
 */
void Task_Init()
{
    xTaskCreate(
        Task1,
        "任务1",
        512,
        NULL,
        4,
        NULL);

    xTaskCreate(
        Task2,
        "Debug任务",
        512,
        NULL,
        2,
        NULL);
}

/**
 * @brief  任务1：LED 控制任务
 * @param  pv: 任务参数（当前未使用）
 * @note   当前实现为 LED 渐变循环，后续可添加更多显示逻辑。
 */
void Task1(void *pv)
{
    float pos_tar = 0;
    float vel_tar = 6;
    float kp = 0;
    float kd = 0.6;
    float torq_tar = 0;
    if (DMJ4310_M1.SendControlCommand("ENABLE") != true)
    {
        usart_printf("Send Control Command failed\r\n");
    };

    for (;;) {
        LED_Loop();
        DMJ4310_M1.UpdateInfoTarget(pos_tar, vel_tar, kp, kd, torq_tar);
        DMJ4310_M1.Print();
        if (DMJ4310_M1.MITController() != true)
        {
            usart_printf("MIT_Controller failed\r\n");
        };
        osDelay(pdMS_TO_TICKS(80));
    }
}

/**
 * @brief  任务2：DEBUG
 * @param  pv: 任务参数（当前未使用）
 * @note   当前为串口调试输出占位，后续可添加通信协议或调试功能。
 */
void Task2(void *pv)
{
    for (;;) {
        CheckDebugState();
        osDelay(pdMS_TO_TICKS(100));
    }
}