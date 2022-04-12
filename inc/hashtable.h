#pragma once
#include <stdint.h>
#include "list.h"
#define HASHTABLE_SIZE 32

typedef struct hashtable {
	linkedList_t * table[HASHTABLE_SIZE];
} hashtable_t;
//
//bool
//hashtable_new(hashtable_t * out);
//
//bool
//hashtable_add(hashtable_t * out, void * value, size_t valueSize, int32_t(hasher(int32_t)));
