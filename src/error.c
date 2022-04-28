#include <stdio.h>
#if !defined(_MSC_VER)
void
errorOut(const char * errorLevel, const char * moduleName, const char * message) {
	fprintf(stderr, "[%s] %s: %s\n", errorLevel, moduleName, message);
}
#endif