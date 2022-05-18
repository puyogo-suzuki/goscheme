#include "list.h"

gserror_t
linkedList_new(linkedList_t ** out, size_t size) {
	*out = (linkedList_t *)reallocarray(NULL, 1, sizeof(linkedList_t *) + size);
	if (*out == NULL) return ERR_OUT_OF_MEMORY;
	(*out)->next = LINKEDLIST_TERMINATOR;
#if _DEBUG
	memset(&((*out)->value), 0, size);
#endif
	return ERR_SUCCESS;
}

gserror_t
linkedList_new3(linkedList_t ** out, void * value, size_t size) {
	*out = (linkedList_t *)reallocarray(NULL, 1, sizeof(linkedList_t *) + size);
	if (*out == NULL) return ERR_OUT_OF_MEMORY;
	(*out)->next = LINKEDLIST_TERMINATOR;
	memcpy(&((*out)->value), value, size);
	return ERR_SUCCESS;
}

gserror_t
linkedList_copy(linkedList_t ** out, linkedList_t * inlist, size_t size) {
	linkedList_t ** writeTo = out;
	linkedList_t * readFrom = inlist;
	while(readFrom != LINKEDLIST_TERMINATOR) {
		*writeTo = (linkedList_t *)reallocarray(NULL, 1, sizeof(linkedList_t *) + size);
		if(*writeTo == NULL) return ERR_OUT_OF_MEMORY;
		(*writeTo)->next = LINKEDLIST_TERMINATOR;
		memcpy(&((*writeTo)->value), &(readFrom->value), size);
		readFrom = readFrom->next;
		writeTo = &((*writeTo)->next);
	}
	return ERR_SUCCESS;
}

gserror_t
linkedList_foreach(linkedList_t * self, gserror_t (action)(void *)) {
	linkedList_t * current = self;
	while(current != LINKEDLIST_TERMINATOR) {
		CHKERROR(action(&(current->value)))
		current = current->next;
	}
	return ERR_SUCCESS;
}

gserror_t
linkedList_add(linkedList_t ** out, void * value, size_t size) {
	linkedList_t * prev = *out;
	CHKERROR(linkedList_new3(out, value, size))
	(*out)->next = prev;
	return ERR_SUCCESS;
}

void
linkedList_free(linkedList_t * out) {
	if (out == LINKEDLIST_TERMINATOR) return;
	linkedList_t * ne = out->next;
	while (ne != LINKEDLIST_TERMINATOR) {
		free(out);
		out = ne;
		ne = ne->next;
	}
	free(out);
}

void *
linkedList_get(linkedList_t * self) {
	return (void *)(&(self->value));
}

bool
linkedList_pop(linkedList_t ** self, void * storage, size_t size) {
	if(*self == NULL) return false;
	if (storage != NULL) {
		void * ret = linkedList_get(*self);
		memcpy(storage, ret, size);
	}
	linkedList_t * prev = *self;
	linkedList_t * n = (*self)->next;
	*self = n;
	free(prev);
	return true;
}

bool
linkedList_search(linkedList_t * self, void ** outValue, void * searchValue, bool(comparer(void *, void *)))
{
	for (linkedList_t * cur = self; cur != NULL; cur = cur->next)
		if (comparer((void *)(&(cur->value)), searchValue)) {
			*outValue = &(cur->value);
			return true;
		}
	return false;
}

void
linkedListAppend_init(linkedListAppend_t * out, linkedList_t * current) {
	out->head = current;
	out->tailnext = &(out->head);
	while (*out->tailnext != LINKEDLIST_TERMINATOR)
		out->tailnext = &((*out->tailnext)->next);
}

gserror_t
linkedListAppend_append(linkedListAppend_t * self, void * value, size_t size) {
	linkedList_t ** ne = self->tailnext;
	CHKERROR(linkedList_new3(ne, value, size))
	self->tailnext = &((*ne)->next);
	return ERR_SUCCESS;
}

bool
linkedListAppend_pop(linkedListAppend_t * self, void * storage, size_t size) {
	if (self->tailnext == &self->head->next) self->tailnext = &self->head;
	return linkedList_pop(&(self->head), storage, size);
}