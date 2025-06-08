//
// Author: Breezeee
// Date: 25-5-28
//

// =============================== 引入头文件 ===============================
#include "led.hpp"
#include "task_manager.hpp"
#include "usart.h"

// =============================== 宏定义区 ===============================
#define WS2812_LowLevel    0xC0     // 0码
#define WS2812_HighLevel   0xF0     // 1码
// =============================== 变量区 ===============================

// =============================== 函数实现 ===============================
/**
 * @brief  控制 WS2812 灯带颜色输出
 * @param  r: 红色分量（0~255）
 * @param  g: 绿色分量（0~255）
 * @param  b: 蓝色分量（0~255）
 * @note   通过 SPI 发送 RGB 颜色数据到 WS2812 灯带，附加空闲位用于刷新
 */
void WS2812_Ctrl(uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t txbuf[24];
    uint8_t res = 0;
    for (int i = 0; i < 8; i++)
    {
        txbuf[7-i]  = (((g>>i)&0x01) ? WS2812_HighLevel : WS2812_LowLevel)>>1;
        txbuf[15-i] = (((r>>i)&0x01) ? WS2812_HighLevel : WS2812_LowLevel)>>1;
        txbuf[23-i] = (((b>>i)&0x01) ? WS2812_HighLevel : WS2812_LowLevel)>>1;
    }
    HAL_SPI_Transmit(&WS2812_SPI_UNIT, &res, 0, 0xFFFF);
    while (WS2812_SPI_UNIT.State != HAL_SPI_STATE_READY);
    HAL_SPI_Transmit(&WS2812_SPI_UNIT, txbuf, 24, 0xFFFF);
    for (int i = 0; i < 100; i++)
    {
        HAL_SPI_Transmit(&WS2812_SPI_UNIT, &res, 1, 0xFFFF);
    }
}

/**
 * @brief  HSV 色彩空间转换为 RGB 色彩
 * @param  h: 色相（Hue，0~360）
 * @param  s: 饱和度（Saturation，0~1）
 * @param  v: 明度（Value，0~1）
 * @retval RGB 结构体（r/g/b 分量 0~255）
 * @note   用于实现亮度恒定的渐变颜色效果
 */
RGB HSV_to_RGB(float h, float s, float v)
{
    float r, g, b;
    int i;
    float f, p, q, t;

    if (s == 0.0f) {
        r = g = b = v;
    } else {
        h /= 60.0f;
        i = (int)h;
        f = h - i;
        p = v * (1.0f - s);
        q = v * (1.0f - s * f);
        t = v * (1.0f - s * (1.0f - f));

        switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        }
    }

    RGB rgb = {
        .r = (uint8_t)(r * 255),
        .g = (uint8_t)(g * 255),
        .b = (uint8_t)(b * 255)
    };
    return rgb;
}

/**
 * @brief  LED 渐变控制循环（流水灯逻辑）
 * @note   每次调用推进色相变化，产生平滑渐变效果
 */
void LED_Loop()
{
    static float hue = 0.0f;

    // hue 取值范围 0~360，用于控制颜色
    hue += 2.0f;
    if (hue >= 360.0f) hue -= 360.0f;

    // 固定饱和度 1.0、亮度 0.3（可调节亮度强度）
    RGB color = HSV_to_RGB(hue, 1.0f, 0.3f);
    WS2812_Ctrl(color.r, color.g, color.b);
}