//
// Author: Breezeee
// Date: 25-6-2
//

#ifndef DEBUG_HPP
#define DEBUG_HPP

// =============================== 调用库 ===============================
#include <cstdint>
// =============================== 变量区 ===============================
enum class DebugState : uint8_t
{
    OK = 0,
    ERROR,
    TIMEOUT,
    INVALID_PARAM,
    NOT_READY,
    UNKNOWN
};

extern DebugState g_current_state;
// =============================== 函数声明 ===============================
void SetDebugState(DebugState state);
const char* DebugStateToString(DebugState state);
void CheckDebugState();
// =============================== 类声明 ===============================


#endif // DEBUG_HPP
