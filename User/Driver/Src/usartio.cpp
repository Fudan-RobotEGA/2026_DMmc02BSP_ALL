//
// Author: Breezeee
// Date: 25-6-9
//


// =============================== 引入头文件 ===============================
#include "usartio.hpp"
#include <stdio.h>
#include "usart.h"
#include "stdarg.h"

// =============================== 宏定义区 ===============================
#define TX_BUF_SIZE 512
// =============================== 变量区 ===============================
uint8_t send_buff[TX_BUF_SIZE];
// =============================== 函数实现 ===============================
/**
 * @brief  格式化打印函数，通过UART DMA发送字符串
 * @param  format: 格式化字符串，类似于printf
 * @param  ...: 可变参数列表，与format匹配
 * @note   用于将调试信息通过USART接口输出到串口终端
 * @retval 无
 */
void usart_printf(const char *format, ...)
{
    va_list args;
    uint32_t length;

    va_start(args, format);
    length = vsnprintf((char *)send_buff, TX_BUF_SIZE, (const char *)format, args);
    va_end(args);
    HAL_UART_Transmit_DMA(&huart1, (uint8_t *)send_buff, length);
}