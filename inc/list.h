#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "winbsdfunc.h"
#include "error.h"

#define LINKEDLIST_TERMINATOR NULL

typedef struct linkedList {
	struct linkedList * next;
	void * value;
} linkedList_t;

typedef struct linkedListAppend {
	struct linkedList * head;
	struct linkedList ** tailnext;
} linkedListAppend_t;

#define linkedList_new2(o, type) linkedList_new(o, sizeof(type))
#define linkedList_new4(o, value, type) linkedList_new3(o, value, sizeof(type))
#define linkedList_add2(o, value, type) linkedList_add(o, value, sizeof(type))
#define linkedList_get2(self, type) (type*)linkedList_get(self);
#define linkedList_pop2(self, storage, type) linkedList_pop(self, storage, sizeof(type))
#define linkedListAppend_append2(self, value, type) linkedListAppend_append(self, value, sizeof(type))
error_t
linkedList_new(linkedList_t ** out, size_t size);
error_t
linkedList_new3(linkedList_t ** out, void * value, size_t size);
error_t
linkedList_add(linkedList_t ** out, void * value, size_t size);
void
linkedList_free(linkedList_t * out);
void *
linkedList_get(linkedList_t * self);
bool
linkedList_pop(linkedList_t ** self, void * storage, size_t size);
void *
linkedList_search(linkedList_t * self, void * searchValue, bool(comparer(void *, void *)));

void
linkedListAppend_init(linkedListAppend_t * out, linkedList_t * current);
void
linkedListAppend_append(linkedListAppend_t * self, void * value, size_t size);