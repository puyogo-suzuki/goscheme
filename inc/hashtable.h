#pragma once
#include <stdint.h>
#include "list.h"
#define HASHTABLE_SIZE 32

typedef struct hashtable {
	linkedList_t * table[HASHTABLE_SIZE];
} hashtable_t;

error_t
hashtable_new(hashtable_t * out);

error_t
hashtable_add(hashtable_t * out, void * value, size_t valueSize, int32_t(hasher(void *)));

void *
hashtable_get(hashtable_t * self, void * value, int32_t(hasher(void *)), bool(comparer(void *, void *)));