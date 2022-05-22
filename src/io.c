#include "io.h"
#if _ESP
#include "esp_cpp_ffi.h"
void
string_write(FILE * f, string_t * str) {
	serialOut2(str->buffer, string_getLength(str));
}

void
string_writeLine(FILE * f, string_t * str) {
	serialOut3(str->buffer, string_getLength(str));
}
#else
void
string_write(FILE * f, string_t * str) {
	fwrite(str->buffer, 1, string_getLength(str), f);
	fflush(f);
}

void
string_writeLine(FILE * f, string_t * str) {
	string_write(f, str);
#if _MSC_VER
	putc('\r', f); // 実は要らん
#endif
	putc('\n', f);
	fflush(f);
}
#endif