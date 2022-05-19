#pragma once
#if _ESP
#include "esp_cpp_ffi.h"
#include "schemeObject_t.h"

DECL_SCHEMEFUNC(schemefunc_esp_lcdDrawPixel);
DECL_SCHEMEFUNC(schemefunc_esp_lcdDrawLine);
DECL_SCHEMEFUNC(schemefunc_esp_lcdFillRect);
DECL_SCHEMEFUNC(schemefunc_esp_lcdFillScreen);
DECL_SCHEMEFUNC(schemefunc_esp_lcdSetCursor);
DECL_SCHEMEFUNC(schemefunc_esp_lcdSetTextColor);
DECL_SCHEMEFUNC(schemefunc_esp_lcdSetTextSize);
DECL_SCHEMEFUNC(schemefunc_esp_lcdDbg);
DECL_SCHEMEFUNC(schemefunc_esp_lcdPrint);
DECL_SCHEMEFUNC(schemefunc_esp_M5Sleep);
DECL_SCHEMEFUNC(schemefunc_esp_M5Update);
DECL_SCHEMEFUNC(schemefunc_esp_buttonAPressp);
DECL_SCHEMEFUNC(schemefunc_esp_buttonBPressp);
DECL_SCHEMEFUNC(schemefunc_esp_buttonCPressp);
DECL_SCHEMEFUNC(schemefunc_esp_accel);
DECL_SCHEMEFUNC(schemefunc_esp_gyro);
DECL_SCHEMEFUNC(schemefunc_esp_temp);

#endif