#include "string_t.h"
#include "winbsdfunc.h"

gserror_t
string_new(string_t * outstring, size_t length) {
    char * buf = (char *)reallocarray(NULL, length + 1, sizeof(char));
    if (buf == NULL) return ERR_OUT_OF_MEMORY;
    outstring->buffer = buf;
    outstring->length = length;
    outstring->buffer[outstring->length] = '\0';
    return ERR_SUCCESS;
}

gserror_t
string_new_shallow(string_t * outstring, char * buf, size_t length) {
#if _DEBUG
    *outstring = (string_t){ buf, length, true };
#else
    *outstring = (string_t){ buf, length };
#endif
    return ERR_SUCCESS;
}

gserror_t
string_new_shallow2(string_t * outstring, char * buf) {
    return string_new_shallow(outstring, buf, strlen(buf));
}

gserror_t
string_new_deep(string_t * outstring, char * buf, size_t length) {
    *outstring = (string_t){(char *)reallocarray(NULL, length + 1, sizeof(char)), length};
    if (outstring->buffer == NULL) {
        free(outstring->buffer);
        return ERR_OUT_OF_MEMORY;
    }
    strncpy(outstring->buffer, buf, length);
    outstring->buffer[length] = '\0';
    return ERR_SUCCESS;
}

gserror_t
string_new_deep2(string_t * outstring, char * buf) {
    return string_new_deep(outstring, buf, strlen(buf));
}

gserror_t
string_copy(string_t * dst, string_t * src){
    return string_new_deep(dst, src->buffer, src->length);
}

gserror_t
string_substring_shallow(string_t * outstring, string_t * src, size_t start, size_t length) {
    if(src->length < start + length) return ERR_OUT_OF_INDEX;
    return string_new_shallow(outstring, &(src->buffer[start]), length);
}

gserror_t
string_substring_deep(string_t * outstring, const string_t * src, size_t start, size_t length) {
    if(src->length < start + length) return ERR_OUT_OF_INDEX;
    return string_new_deep(outstring, &(src->buffer[start]), length);
}

gserror_t
string_getAt(string_t * s, size_t index, char * outch) {
    if(s->length <= index) return ERR_OUT_OF_INDEX;
    *outch = s->buffer[index];
    return ERR_SUCCESS;
}

int32_t string_hash(string_t * self)
{
    char ch;
    int32_t result = 0;
    for (size_t i = 0; string_getAt(self, i, &ch); ++i)
        result ^= ch;
    return result;
}

gserror_t
string_overWrite(string_t * dst, const string_t * src, size_t start) {
    if (start + src->length > dst->length) {
        char * buf = (char *)reallocarray(dst->buffer, start + src->length, sizeof(char));
        if (buf == NULL) return ERR_OUT_OF_MEMORY;
        dst->buffer = buf;
        dst->length = start + src->length;
    }
    memcpy(&dst->buffer[start], src->buffer, src->length);
    return true;
}

bool
string_equals(string_t * a, string_t * b) {
    if(a->length != b->length) return false;
    return strncmp(a->buffer, b->buffer, a->length) == 0;
}

bool
string_equals2(string_t * a, char * b, int length) {
    if(a->length != length) return false;
    return strncmp(a->buffer, b, length) == 0;
}

size_t
string_getLength(string_t * s) {
    return s->length;
}

gserror_t
string_parseInt(string_t * s, int32_t * outi) {
    char ch = '\0';
    int ret = 0;
    if (string_getAt(s, 0, &ch) != ERR_SUCCESS) return ERR_INTEGER_FORMAT;
    bool isMinus = ch == '-';
    for (int i = isMinus ? 1 : 0; string_getAt(s, i, &ch) == ERR_SUCCESS; ++i) {
        if (ch < '0' || ch > '9') return ERR_INTEGER_FORMAT;
        ret = (ret * 10) + (int32_t)(ch - '0');
    }
    *outi = isMinus ? -ret : ret;
    return ERR_SUCCESS;
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

gserror_t
stringBuilder_new(stringBuilder_t * outsb) {
    outsb->buffer = (char *)reallocarray(NULL, sizeof(char), STRINGBUILDER_INITIAL_SIZE);
    if(outsb->buffer == NULL) return ERR_OUT_OF_MEMORY;
    outsb->bufferSize = STRINGBUILDER_INITIAL_SIZE;
    outsb->length = 0;
    outsb->buffer[0] = '0';
    return ERR_SUCCESS;
}

gserror_t
stringBuilder_append(stringBuilder_t * outsb, char * str, size_t length) {
    if(length + outsb->length + 1 > outsb->bufferSize) {
        size_t increaseSize = 0;
        if(outsb->bufferSize < STRINGBUILDER_INCREASE_SIZE)
            increaseSize = outsb->bufferSize;
        else
            increaseSize = STRINGBUILDER_INCREASE_SIZE;
        if(length + outsb->length + 1 > outsb->bufferSize + STRINGBUILDER_INCREASE_SIZE)
            increaseSize = length + outsb->length + 1 - outsb->bufferSize;
        char * p = (char *)reallocarray(outsb->buffer, sizeof(char), outsb->bufferSize + increaseSize);
        if(p == NULL) return ERR_OUT_OF_MEMORY;
        outsb->buffer = p;
        outsb->bufferSize += increaseSize;
    }
    for(int i = 0; i < length; ++i)
        outsb->buffer[outsb->length + i] = str[i];
    outsb->length += length;
    outsb->buffer[outsb->length] = '\0';
    return ERR_SUCCESS;
}

gserror_t
stringBuilder_toString(string_t * outstr, stringBuilder_t * insb) {
    return string_new_deep(outstr, insb->buffer, insb->length);
}

void
stringBuilder_free(stringBuilder_t * sb) {
    free(sb->buffer);
#if _DEBUG
    sb->buffer = NULL;
    sb->bufferSize = 0;
    sb->length = 0;
#endif
}