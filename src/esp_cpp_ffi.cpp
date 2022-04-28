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
    String str(s, length);
    Serial.print(str);
}
void
serialOut3(const char * s, int length) {
    String str(s, length);
    Serial.println(str);
}
void
serialPutc(char ch){
    Serial.println(ch);
}
void
serialPrintf(const char * format, ...) {
    va_list list;
    va_start(list, format);
    Serial.printf(format, list);
    va_end(list);
}
}