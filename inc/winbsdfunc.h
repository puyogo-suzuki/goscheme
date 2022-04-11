#pragma once
#include <stdlib.h>

#if _MSC_VER || __APPLE__
// Linux and *BSD(exclude macOS!!) only
void *
reallocarray(void * ptr, size_t nmemb, size_t size);
#endif