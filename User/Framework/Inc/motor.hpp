//
// Author: Breezeee
// Date: 25-5-29
//

#ifndef MOTOR_HPP
#define MOTOR_HPP

// =============================== 调用库 ===============================
#include "stm32h7xx.h"
#include <sys/_stdint.h>
#include "stm32h7xx_hal_fdcan.h"
#include "usart.h"
#include "canio.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "debug.hpp"

// =============================== 变量区 ===============================
constexpr uint8_t MIT_P_BIT_NUM = 16;
constexpr uint8_t MIT_V_BIT_NUM = 12;
constexpr uint8_t MIT_T_BIT_NUM = 12;
constexpr uint8_t MIT_KP_BIT_NUM = 12;
constexpr uint8_t MIT_KD_BIT_NUM = 12;

constexpr float MIT_P_MIN = -12.5f;
constexpr float MIT_P_MAX = 12.5f;
constexpr float MIT_V_MIN = -30.0f;
constexpr float MIT_V_MAX = 30.0f;
constexpr float MIT_T_MIN = -10.0f;
constexpr float MIT_T_MAX = 10.0f;
constexpr float MIT_KP_MIN = 0.0f;
constexpr float MIT_KP_MAX = 500.0f;
constexpr float MIT_KD_MIN = 0.0f;
constexpr float MIT_KD_MAX = 5.0f;

// =============================== 函数声明 ===============================
float Uint2Float_ForDMJ4310(int x_int, float x_min, float x_max, int bits);
int Float2Uint_ForDMJ4310(float x, float x_min, float x_max, int bits);

// =============================== 类声明 ===============================
class DMJ4310_CTRL
{
public:
    uint32_t can_id_send_;
    uint32_t can_id_receive_;
    FDCAN_HandleTypeDef *fdcan_;

    DMJ4310_CTRL(FDCAN_HandleTypeDef *fdcan = nullptr,
                 uint32_t can_id_send = 0,
                 uint32_t can_id_receive = 0)
        : can_id_send_(can_id_send),
          can_id_receive_(can_id_receive),
          fdcan_(fdcan),
          err_(0), id_part_(0),
          pos_(0.0f), vel_(0.0f), torq_(0.0f),
          t_mos_(0), t_rotor_(0),
          pos_int_(0), vel_int_(0), torq_int_(0),
          target_pos_(0.0f), target_vel_(0.0f),
          target_KP_(0.0f), target_KD_(0.0f), target_torq_(0.0f)
    {}

    void UpdateInfoEncoder(const uint8_t *rx_data);
    void UpdateInfoTarget(float pos, float vel, float KP, float KD, float torq);
    bool SendControlCommand(const char *cmd_name);
    bool MITController();
    void Print() const;

    // Getter
    uint8_t getIDPart() const { return id_part_; }
    uint8_t getError() const { return err_; }
    float getPos() const { return pos_; }
    float getVel() const { return vel_; }
    float getTorq() const { return torq_; }
    uint8_t getTMos() const { return t_mos_; }
    uint8_t getTRotor() const { return t_rotor_; }

private:
    // 电机状态
    uint8_t err_;
    uint8_t id_part_;
    uint16_t pos_int_;
    uint16_t vel_int_;
    uint16_t torq_int_;
    float pos_;
    float vel_;
    float torq_;
    uint8_t t_mos_;
    uint8_t t_rotor_;

    // 期望值
    float target_pos_;
    float target_vel_;
    float target_KP_;
    float target_KD_;
    float target_torq_;
};


typedef enum
{
    MIT = 0x000,
    PV  = 0x100,
    V   = 0x200
}DMJ4310_CTRL_Mode;

extern DMJ4310_CTRL DMJ4310_M1;
extern DMJ4310_CTRL DMJ4310_M2;
extern DMJ4310_CTRL DMJ4310_M3;
extern DMJ4310_CTRL DMJ4310_M4;

#endif // MOTOR_HPP
