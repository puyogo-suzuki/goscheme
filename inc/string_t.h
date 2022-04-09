#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct string {
    char * buffer;
    size_t length;
#if _DEBUG
    bool isShallow;
#endif
} string_t;
// buffer[length]はデバッガのために'\0'を入れることがあるが，必ずしも入っているわけではない．

bool string_new(string_t * outstring, size_t length);
bool string_new_shallow(string_t * outstring, char * buf, size_t length);
bool string_new_shallow2(string_t * outstring, char * buf);
bool string_new_deep(string_t * outstring, char * buf, size_t length);
bool string_new_deep2(string_t * outstring, char * buf);
bool string_copy(string_t * restrict dst, string_t * restrict src);
bool string_substring_shallow(string_t * outstring, string_t * src, size_t start, size_t length);
bool string_substring_deep(string_t * restrict outstring, const string_t * restrict src, size_t start, size_t length);
bool string_getAt(string_t * s, size_t index, char * outch);
bool string_overWrite(string_t * restrict dst, const string_t * restrict src, size_t start);
size_t string_getLength(string_t * s);
bool string_parseInt(string_t * s, int32_t * outi);
void string_default(string_t * s);
void string_free(string_t * s);