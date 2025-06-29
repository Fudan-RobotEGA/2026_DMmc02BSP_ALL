//
// Author: Breezeee
// Date: 25-5-29
//


// =============================== 引入头文件 ===============================
#include "motor.hpp"
#include <string.h>
#include "canio.hpp"
#include "usartio.hpp"
#include "fdcan.h"

// =============================== 宏定义区 ===============================

// =============================== 变量区 ===============================
DMJ4310_CTRL DMJ4310_M1(&hfdcan1, CAN_DMJ4310_M1_SEND_ID, CAN_DMJ4310_M1_RECEIVE_ID);
DMJ4310_CTRL DMJ4310_M2(&hfdcan1, CAN_DMJ4310_M2_SEND_ID, CAN_DMJ4310_M2_RECEIVE_ID);
DMJ4310_CTRL DMJ4310_M3(&hfdcan1, CAN_DMJ4310_M3_SEND_ID, CAN_DMJ4310_M3_RECEIVE_ID);
DMJ4310_CTRL DMJ4310_M4(&hfdcan1, CAN_DMJ4310_M4_SEND_ID, CAN_DMJ4310_M4_RECEIVE_ID);

// =============================== 函数实现 ===============================
/**
 * @brief  无符号整数转换为浮点数
 * @param  x_int: 需要转换的整数值
 * @param  x_min: 浮点数对应的最小值
 * @param  x_max: 浮点数对应的最大值
 * @param  bits: 量化位数（如8位、12位）
 * @note   用于将CAN总线或寄存器传输的整型数据映射回物理量
 * @retval 转换后的浮点数值
 */
float Uint2Float_ForDMJ4310(int x_int, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return ((float)x_int) * span / ((float)((1 << bits) - 1)) + offset;
}

/**
 * @brief  浮点数转换为无符号整数
 * @param  x: 需要转换的浮点数值
 * @param  x_min: 浮点数对应的最小值
 * @param  x_max: 浮点数对应的最大值
 * @param  bits: 量化位数（如8位、12位）
 * @note   用于将物理量映射到CAN总线或寄存器的整型数据
 * @retval 转换后的整数值
 */
int Float2Uint_ForDMJ4310(float x, float x_min, float x_max, int bits)
{
    float span = x_max - x_min;
    float offset = x_min;
    return (int)(((x - offset) * ((float)((1 << bits) - 1))) / span);
}

/**
 * @brief  解析DMJ4310电机的反馈数据帧
 * @param  rx_data: 指向接收数据的8字节数组
 * @note   根据电机的反馈协议，逐字节解析ERR、ID、位置、速度、转矩及温度信息。
 *         - D[0]：高4位为ERR（状态码），低4位为ID（电机ID的低8位）。
 *                ERR状态：
 *                  0 - 无使能
 *                  1 - 使能
 *                  2 - 超压
 *                  3 - 欠压
 *                  4 - 一次压
 *                  5 - 二次压
 *                  6 - MOS过温
 *                  7 - 电机绕组过温
 *                  8 - 电机绕组过温
 *                  9 - 欠压
 *                  A/B/C/D/E - 过流、MOS过温、电机绕组过温、通讯丢失
 *         - D[1]/D[2]：位置POS（16位，单位rad，物理量通过Uint2Float转换）
 *         - D[3]/D[4]：速度VEL（高12位，单位：rad/s，物理量通过Uint2Float转换）
 *         - D[4]/D[5]：转矩TORQ（高12位，单位：Nm，物理量通过Uint2Float转换）
 *         - D[6]：MOS平均温度（单位：℃）
 *         - D[7]：电机绕组平均温度（单位：℃）
 * @retval None
 */
void DMJ4310_CTRL::UpdateInfoEncoder(const uint8_t *rx_data)
{
    err_ = (rx_data[0] >> 4) & 0x0F;
    id_part_ = rx_data[0] & 0x0F;
    pos_int_ = (rx_data[1] << 8) | rx_data[2];
    vel_int_ = (rx_data[3] << 4) | ((rx_data[4] >> 4) & 0x0F);
    torq_int_ = ((rx_data[4] & 0x0F) << 8) | rx_data[5];
    t_mos_ = rx_data[6];
    t_rotor_ = rx_data[7];

    pos_ = Uint2Float_ForDMJ4310(pos_int_, MIT_P_MIN, MIT_P_MAX, MIT_P_BIT_NUM);
    vel_ = Uint2Float_ForDMJ4310(vel_int_, MIT_V_MIN, MIT_V_MAX, MIT_V_BIT_NUM);
    torq_ = Uint2Float_ForDMJ4310(torq_int_, MIT_T_MIN, MIT_T_MAX, MIT_T_BIT_NUM);
}

/**
 * @brief  更新电机控制目标值
 * @param  pos:     目标位置（单位：rad）
 * @param  vel:     目标速度（单位：rad/s）
 * @param  KP:      位置环增益（单位：N/r）
 * @param  KD:      速度环增益（单位：Ns/r）
 * @param  torq:    目标转矩（单位：Nm）
 * @note   将目标值保存到对象内部，用于后续MIT控制器发送
 * @retval None
 */
void DMJ4310_CTRL::UpdateInfoTarget(float pos, float vel, float KP, float KD, float torq)
{
    target_pos_ = pos;
    target_vel_ = vel;
    target_KP_ = KP;
    target_KD_ = KD;
    target_torq_ = torq;
}

/**
 * @brief  向电机发送控制指令（如启用或失能）
 * @param  cmd 控制命令类型（DMJ4310Command::ENABLE 或 DISABLE）
 * @note
 * - 该函数通过 CAN 总线向电机发送 8 字节控制帧，帧尾携带指令字节。
 * - 命令发送后，函数将轮询电机状态（通过 getError()）判断是否达到目标状态：
 *   - ENABLE 期望 error 状态变为 1；
 *   - DISABLE 期望 error 状态变为 0。
 * - 在发送失败或超过最大重试次数前，最多尝试 1000 次，每次延时 1ms。
 * @retval true  控制命令发送成功，且目标状态达到预期
 * @retval false 发送失败，或超时仍未达到目标状态
 */
bool DMJ4310_CTRL::SendControlCommand(DMJ4310Command cmd)
{
    if (fdcan_ == nullptr) return false;

    uint8_t cmd_byte = static_cast<uint8_t>(cmd);
    uint8_t target_err = (cmd == DMJ4310Command::ENABLE) ? 1 : 0;

    uint8_t tx_data[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, cmd_byte};

    FDCAN_TxHeaderTypeDef txHeader;
    txHeader.Identifier = can_id_send_;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = FDCAN_DLC_BYTES_8;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    const uint32_t max_retry = 1000;
    uint32_t retry_count = 0;

    while (getError() != target_err && retry_count < max_retry)
    {
        if (HAL_FDCAN_AddMessageToTxFifoQ(fdcan_, &txHeader, tx_data) != HAL_OK)
            return false;
        HAL_Delay(1);
        retry_count++;
    }

    return getError() == target_err;
}


/**
 * @brief  发送MIT模式的电机控制指令
 * @note   将目标位置、速度、KP、KD、转矩等物理量通过量化转换为CAN报文，
 *         并通过CAN总线发送至电机控制器。
 * @retval bool:
 *         - true: 发送成功
 *         - false: 发送失败（例如CAN发送队列满或其他错误）
 */
bool DMJ4310_CTRL::MITController()
{
    if (fdcan_ == nullptr) return false;

    uint16_t pos_int = Float2Uint_ForDMJ4310(target_pos_, MIT_P_MIN, MIT_P_MAX, MIT_P_BIT_NUM);
    uint16_t vel_int = Float2Uint_ForDMJ4310(target_vel_, MIT_V_MIN, MIT_V_MAX, MIT_V_BIT_NUM);
    uint16_t kp_int  = Float2Uint_ForDMJ4310(target_KP_, MIT_KP_MIN, MIT_KP_MAX, MIT_KP_BIT_NUM);
    uint16_t kd_int  = Float2Uint_ForDMJ4310(target_KD_, MIT_KD_MIN, MIT_KD_MAX, MIT_KD_BIT_NUM);
    uint16_t torq_int = Float2Uint_ForDMJ4310(target_torq_, MIT_T_MIN, MIT_T_MAX, MIT_T_BIT_NUM);

    uint8_t tx_data[8];
    tx_data[0] = (pos_int >> 8) & 0xFF;
    tx_data[1] = pos_int & 0xFF;
    tx_data[2] = (vel_int >> 4) & 0xFF;
    tx_data[3] = ((vel_int & 0xF) << 4) | ((kp_int >> 8) & 0x0F);
    tx_data[4] = kp_int & 0xFF;
    tx_data[5] = (kd_int >> 4) & 0xFF;
    tx_data[6] = ((kd_int & 0xF) << 4) | ((torq_int >> 8) & 0x0F);
    tx_data[7] = torq_int & 0xFF;

    FDCAN_TxHeaderTypeDef txHeader;
    txHeader.Identifier = can_id_send_ + (uint16_t)DMJ4310CtrlMode::MIT;
    txHeader.IdType = FDCAN_STANDARD_ID;
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = FDCAN_DLC_BYTES_8;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_CLASSIC_CAN;
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(fdcan_, &txHeader, tx_data) != HAL_OK) return false;
    return true;
}

/**
 * @brief  打印电机当前状态信息
 * @note   通过串口输出电机的CAN接收ID部分、错误状态、位置、速度、转矩、MOS平均温度、电机绕组平均温度。
 *         - id_part_: 当前反馈的电机CAN ID
 *         - ERR: 错误状态（解析自ERR字段，高4位）
 *         - POS: 当前电机位置（单位：根据协议约定）
 *         - VEL: 当前电机速度（单位：根据协议约定）
 *         - TORQ: 当前电机转矩（单位：根据协议约定）
 *         - T_MOS: MOS平均温度（单位：℃）
 *         - T_Rotor: 电机绕组平均温度（单位：℃）
 * @retval None
 */
void DMJ4310_CTRL::Print() const
{
    // usart_printf("CAN_ID_Receive=%d, ERR=%d, POS=%f, VEL=%f, TORQ=%f, T_MOS=%d, T_Rotor=%d\r\n",
    //              id_part_, err_, pos_, vel_, torq_, t_mos_, t_rotor_);

    usart_printf("%f,%f\n", pos_, vel_);
}
