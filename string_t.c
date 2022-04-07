#include "string_t.h"

bool
string_new_shallow(string_t * outstring, char * buf, int length) {
    *outstring = (string_t){buf, length};
    return true;
}

bool
string_new_shallow2(string_t * outstring, char * buf) {
    return string_new_shallow(outstring, buf, strlen(buf));
}

bool
string_new_deep(string_t * outstring, char * buf, int length) {
    *outstring = (string_t){(char *)malloc(sizeof(char) * length), length};
    if(outstring->buffer == NULL) return false; 
    strncpy(outstring->buffer, buf, length);
    return true;
}

bool
string_new_deep2(string_t * outstring, char * buf) {
    return string_new_deep(outstring, buf, strlen(buf));
}

bool string_copy(string_t * dst, string_t * src){
    *dst = (string_t){(char *)malloc(sizeof(char) * src->length), src->length};
    return dst->buffer != NULL;
}
bool
string_substring_shallow(string_t * outstring, string_t * src, int start, int length) {
    if(src->length > start + length) return false;
    return string_new_shallow(outstring, &(src->buffer[start]), length);
}

bool
string_substring_deep(string_t * outstring, string_t * src, int start, int length) {
    if(src->length > start + length) return false;
    return string_new_deep(outstring, &(src->buffer[start]), length);
}

