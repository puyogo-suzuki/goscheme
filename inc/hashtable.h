#pragma once
#include <stdint.h>
#include "list.h"
#define HASHTABLE_SIZE 32

typedef struct hashtable {
	linkedList_t * table[HASHTABLE_SIZE];
} hashtable_t;

gserror_t
hashtable_new(hashtable_t * out);

void
hashtable_free(hashtable_t * self);

gserror_t
hashtable_add(hashtable_t * out, void * value, size_t valueSize, int32_t(hasher(void *)));

bool
hashtable_get(hashtable_t * self, void ** outValue, void * value, int32_t(hasher(void *)), bool(comparer(void *, void *)));

gserror_t
hashtable_copy(hashtable_t * dst, hashtable_t * src, size_t size);

gserror_t
hashtable_foreach(hashtable_t * self, gserror_t (action)(void *));
