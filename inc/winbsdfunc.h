#pragma once
#include <stdlib.h>

#if _MSC_VER
void *
reallocarray(void * ptr, size_t nmemb, size_t size);
#endif