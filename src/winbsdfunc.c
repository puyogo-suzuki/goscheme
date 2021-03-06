#include "winbsdfunc.h"

#if _MSC_VER || __APPLE__
void *
reallocarray(void * ptr, size_t nmemb, size_t size) {
	return nmemb * size / size != nmemb ? NULL : realloc(ptr, nmemb * size);
}
#endif