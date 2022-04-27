#include <stdio.h>
#if !defined(_MSC_VER)
void
errorOut(char * errorLevel, char * moduleName, char * message) {
	fprintf(stderr, "[%s] %s: %s\n", errorLevel, moduleName, message);
}
#endif