#include "error.h"
bool error_OOMRecoverByFreeing = true;

#if _ESP
#include "esp_cpp_ffi.h"
void
errorOut(const char * errorLevel, const char * moduleName, const char * message) {
	serialOut4("[");
	serialOut4(errorLevel);
	serialOut4("] ");
	serialOut4(moduleName);
	serialOut4(": ");
	serialOut(message);
}

#else
#include <stdio.h>
void
errorOut(const char * errorLevel, const char * moduleName, const char * message) {
	fprintf(stderr, "[%s] %s: %s\n", errorLevel, moduleName, message);
}
#endif