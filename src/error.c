#include "error.h"

#if _ESP
#include "esp_cpp_ffi.h"
void
errorOut(const char * errorLevel, const char * moduleName, const char * message) {
	serialPrintf("[%s] %s: %s\n", errorLevel, moduleName, message);
}

#else
#include <stdio.h>
void
errorOut(const char * errorLevel, const char * moduleName, const char * message) {
	fprintf(stderr, "[%s] %s: %s\n", errorLevel, moduleName, message);
}
#endif