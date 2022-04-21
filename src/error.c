#include <stdio.h>

void
errorOut(char * errorLevel, char * moduleName, char * message) {
	fprintf(stderr, "[%s] %s: %s\n", errorLevel, moduleName, message);
}