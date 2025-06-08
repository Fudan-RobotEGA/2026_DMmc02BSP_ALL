//
// Author: Breezeee
// Date: 25-5-28
//

#ifndef LED_HPP
#define LED_HPP

// =============================== 调用库 ===============================
#include "main.h"
// =============================== 变量区 ===============================
#define WS2812_SPI_UNIT     hspi6
extern SPI_HandleTypeDef WS2812_SPI_UNIT;

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGB;
// =============================== 函数声明 ===============================
void WS2812_Ctrl(uint8_t r, uint8_t g, uint8_t b);
void LED_Loop();
RGB HSV_to_RGB(float h, float s, float v);
// =============================== 类声明 ===============================


#endif // LED_HPP
