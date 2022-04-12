#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "error.h"

typedef struct string {
    char * buffer;
    size_t length;
#if _DEBUG
    bool isShallow;
#endif
} string_t;
// buffer[length]はデバッガのために'\0'を入れることがあるが，必ずしも入っているわけではない．

#define STRINGBUILDER_INITIAL_SIZE 64
#define STRINGBUILDER_INCREASE_SIZE 256
typedef struct stringBuilder {
    char * buffer;
    size_t length;
    size_t bufferSize;
} stringBuilder_t;

#define string_equals3(a, b) string_equals2(a, b, strlen(b))
#define stringBuilder_append2(outsb, str) stringBuilder_append(outsb, (str)->buffer, string_getLength(str))

error_t string_new(string_t * outstring, size_t length);
error_t string_new_shallow(string_t * outstring, char * buf, size_t length);
error_t string_new_shallow2(string_t * outstring, char * buf);
error_t string_new_deep(string_t * outstring, char * buf, size_t length);
error_t string_new_deep2(string_t * outstring, char * buf);
error_t string_copy(string_t * dst, string_t * src);
error_t string_substring_shallow(string_t * outstring, string_t * src, size_t start, size_t length);
error_t string_substring_deep(string_t * outstring, const string_t * src, size_t start, size_t length);
error_t string_getAt(string_t * s, size_t index, char * outch);
int32_t string_hash(string_t * self);
error_t string_overWrite(string_t * dst, const string_t *  src, size_t start);
bool string_equals(string_t * a, string_t * b);
bool string_equals2(string_t * a, char * b, int length);
size_t string_getLength(string_t * s);
error_t string_parseInt(string_t * s, int32_t * outi);
void string_default(string_t * s);
void string_free(string_t * s);

error_t stringBuilder_new(stringBuilder_t * outsb);
error_t stringBuilder_append(stringBuilder_t * outsb, char * str, size_t length);
error_t stringBuilder_toString(string_t * outstr, stringBuilder_t * insb);
void stringBuilder_free(stringBuilder_t * sb);