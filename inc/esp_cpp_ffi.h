#pragma once
#if _ESP
#if __cplusplus
extern "C" {
#endif
void
serialOut(const char * str);
void
serialOut2(const char * s, int length);
void
serialOut3(const char * s, int length);
void
serialPutc(char ch);
void
serialPrintf(const char * format, ...);
#if __cplusplus
}
#endif
#endif