//
// Author: Breezeee
// Date: 25-6-2
//


// =============================== 引入头文件 ===============================
#include "debug.hpp"
#include "usartio.hpp"

// =============================== 宏定义区 ===============================

// =============================== 变量区 ===============================
DebugState g_current_state = DebugState::OK;  // 全局状态
// =============================== 函数实现 ===============================
void SetDebugState(DebugState state)
{
    g_current_state = state;
}

const char* DebugStateToString(DebugState state)
{
    switch (state)
    {
    case DebugState::OK:
        return "OK";
    case DebugState::ERROR:
        return "ERROR";
    case DebugState::TIMEOUT:
        return "TIMEOUT";
    case DebugState::INVALID_PARAM:
        return "INVALID_PARAM";
    case DebugState::NOT_READY:
        return "NOT_READY";
    default:
        return "UNKNOWN";
    }
}

void CheckDebugState()
{
    if (g_current_state != DebugState::OK)
    {
        usart_printf("[DEBUG] Error State: %s \n", DebugStateToString(g_current_state));
    }
}