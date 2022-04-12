#include "io.h"


void
string_write(FILE * f, string_t * str) {
	char cur;
	for (int i = 0; string_getAt(str, i, &cur) == ERR_SUCCESS; ++i) {
		putc(cur, f);
	}
}

void
string_writeLine(FILE * f, string_t * str) {
	string_write(f, str);
#if _MSC_VER
	putc('\r', f); // ŽÀ‚Í—v‚ç‚ñ
#endif
	putc('\n', f);
}