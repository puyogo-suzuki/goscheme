#include "esp_cpp_ffi.h"
#include <M5Core2.h>
#include <stdarg.h>
extern "C" {
void
serialOut(const char * str) {
    Serial.println(str);
}
void
serialOut2(const char * s, int length) {
    Serial.write(s, length);
}
void
serialOut3(const char * s, int length) {
    Serial.write(s, length);
    Serial.println();
}
void
serialOut4(const char * str) {
    Serial.print(str);
}
void
serialPutc(char ch){
    Serial.println(ch);
}
void
serialPrintf(const char * format, ...) {
    // va_list list;
    // va_start(list, format);
    // Serial.printf(format, list);
    // va_end(list);
}

void
lcdDrawPixel(int32_t x, int32_t y, uint32_t color) {
    M5.Lcd.drawPixel(x, y, color);
}
void
lcdDrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color) {
    M5.Lcd.drawLine(x0, y0, x1, y1, color);
}
void
lcdFillRect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    M5.Lcd.fillRect(x,y,w,h,color);
}
void
lcdFillScreen(uint32_t color) {
    M5.Lcd.fillScreen(color);
}
void
lcdSetCursor(uint32_t x, uint32_t y) {
    M5.Lcd.setCursor(x, y);
}
void
lcdSetTextColor(uint32_t color, uint32_t background) {
    M5.Lcd.setTextColor(color, background);
}
void
lcdSetTextSize(uint32_t size) {
    M5.Lcd.setTextSize(size);
}
void
lcdPrint(const char * str) {
    M5.Lcd.print(str);
}

void
M5Sleep(uint32_t sec) {
    delay(sec);
}

void
M5Update(void) {
    M5.update();
}

bool
buttonAIsPressed(void) {
    return M5.BtnA.isPressed();
}
bool
buttonBIsPressed(void) {
    return M5.BtnB.isPressed();
}
bool
buttonCIsPressed(void) {
    return M5.BtnC.isPressed();
}
void
accelGetAccelData(float * x, float * y, float * z){
    M5.IMU.getAccelData(x, y, z);
}
void
gyroGetGyroData(float * x, float * y, float *z){
    M5.IMU.getGyroData(x, y, z);
}
void
gyroGetTempData(float * t) {
    M5.IMU.getTempData(t);
}
}