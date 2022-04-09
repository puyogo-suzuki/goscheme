#include "string_t.h"
#include "winbsdfunc.h"

bool
string_new(string_t * outstring, size_t length) {
    char * buf = (char *)reallocarray(NULL, length + 1, sizeof(char));
    if (buf == NULL) return false;
    outstring->buffer = buf;
    outstring->length = length;
    outstring->buffer[outstring->length] = '\0';
    return true;
}

bool
string_new_shallow(string_t * outstring, char * buf, size_t length) {
#if _DEBUG
    *outstring = (string_t){ buf, length, true };
#else
    *outstring = (string_t){ buf, length };
#endif
    return true;
}

bool
string_new_shallow2(string_t * outstring, char * buf) {
    return string_new_shallow(outstring, buf, strlen(buf));
}

bool
string_new_deep(string_t * outstring, char * buf, size_t length) {
    *outstring = (string_t){(char *)reallocarray(NULL, length + 1, sizeof(char)), length};
    if (outstring->buffer == NULL) {
        free(outstring->buffer);
        return false;
    }
    strncpy(outstring->buffer, buf, length);
    outstring->buffer[length] = '\0';
    return true;
}

bool
string_new_deep2(string_t * outstring, char * buf) {
    return string_new_deep(outstring, buf, strlen(buf));
}

bool
string_copy(string_t * dst, string_t * src){
    return string_new_deep(dst, src->buffer, src->length);
}

bool
string_substring_shallow(string_t * outstring, string_t * src, size_t start, size_t length) {
    if(src->length < start + length) return false;
    return string_new_shallow(outstring, &(src->buffer[start]), length);
}

bool
string_substring_deep(string_t * outstring, const string_t * src, size_t start, size_t length) {
    if(src->length < start + length) return false;
    return string_new_deep(outstring, &(src->buffer[start]), length);
}

bool
string_getAt(string_t * s, size_t index, char * outch) {
    if(s->length <= index) return false;
    *outch = s->buffer[index];
    return true;
}

bool
string_overWrite(string_t * dst, const string_t * src, size_t start) {
    if (start + src->length > dst->length) {
        char * buf = (char *)reallocarray(dst->buffer, start + src->length, sizeof(char));
        if (buf == NULL) return false;
        dst->buffer = buf;
        dst->length = start + src->length;
    }
    memcpy(&dst->buffer[start], src->buffer, src->length);
    return true;
}

size_t
string_getLength(string_t * s) {
    return s->length;
}

bool
string_parseInt(string_t * s, int32_t * outi) {
    char ch = '\0';
    int ret = 0;
    if (!string_getAt(s, 0, &ch)) return false;
    bool isMinus = ch == '-';
    for (int i = isMinus ? 1 : 0; string_getAt(s, i, &ch); ++i) {
        if (ch < '0' || ch > '9') return false;
        ret = (ret * 10) + (int32_t)(ch - '0');
    }
    *outi = isMinus ? -ret : ret;
    return true;
}

void
string_default(string_t * s) {
    s->buffer = NULL;
#if _DEBUG
    s->isShallow = false;
#endif
    s->length = 0;
}

void
string_free(string_t * s) {
#if _DEBUG
    if (s->isShallow) {
        *((int*)NULL) = 32; // throw error.
    }
#endif
    free(s->buffer);
    string_default(s);
}