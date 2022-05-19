#pragma once
#if _ESP
#include <stdint.h>
#include <stdbool.h>
#if __cplusplus
extern "C" {
#endif
extern VL53L0X tofSensor;
void
serialOut(const char * str);
void
serialOut2(const char * s, int length);
void
serialOut3(const char * s, int length);
void
serialOut4(const char * str);
void
serialPutc(char ch);
void
serialPrintf(const char * format, ...);
void
lcdDrawPixel(int32_t x, int32_t y, uint32_t color);
void
lcdDrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color);
void
lcdFillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
void
lcdFillScreen(uint32_t color);
void
lcdSetCursor(uint32_t x, uint32_t y);
void
lcdSetTextColor(uint32_t color, uint32_t background);
void
lcdSetTextSize(uint32_t size);
void
lcdPrint(const char * str);
void
M5Sleep(uint32_t sec);
void
M5Update(void);
bool
buttonAIsPressed(void);
bool
buttonBIsPressed(void);
bool
buttonCIsPressed(void);
void
accelGetAccelData(float * x, float * y, float * z);
void
gyroGetGyroData(float * x, float * y, float *z);
void
gyroGetTempData(float * t);
#if __cplusplus
}
#endif
#endif