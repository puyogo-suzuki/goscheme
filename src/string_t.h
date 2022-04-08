#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct string{
    char * buffer;
    int32_t length;
} string_t;
bool string_new_shallow(string_t * outstring, char * buf, int length);
bool string_new_shallow2(string_t * outstring, char * buf);
bool string_new_deep(string_t * outstring, char * buf, int length);
bool string_new_deep2(string_t * outstring, char * buf);
bool string_copy(string_t * dst, string_t * src);
bool string_substring_shallow(string_t * outstring, string_t * src, int start, int length);
bool string_substring_deep(string_t * outstring, string_t * src, int start, int length);