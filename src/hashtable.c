#include <hashtable.h>

gserror_t
hashtable_new(hashtable_t * out) {
	for (int i = 0; i < HASHTABLE_SIZE; ++i) {
		out->table[i] = NULL;
	}
	return ERR_SUCCESS;
}

void
hashtable_free(hashtable_t * self) {
	for (int i = 0; i < HASHTABLE_SIZE; ++i) {
		linkedList_free(self->table[i]);
	}
}

gserror_t
hashtable_add(hashtable_t * out, void * value, size_t valueSize, int32_t(hasher(void *))) {
	return linkedList_add(&(out->table[hasher(value) % HASHTABLE_SIZE]), value, valueSize);
}

bool
hashtable_get(hashtable_t * self, void ** outValue, void * value, int32_t(hasher(void *)), bool(comparer(void *, void *))) {
	return linkedList_search(self->table[hasher(value) % HASHTABLE_SIZE], outValue, value, comparer);
}


gserror_t
hashtable_copy(hashtable_t * dst, hashtable_t * src, size_t size) {
	for(int i = 0; i < HASHTABLE_SIZE; ++i)
		CHKERROR(linkedList_copy(&(dst->table[i]), src->table[i], size))
	return ERR_SUCCESS;
}

gserror_t
hashtable_foreach(hashtable_t * self, gserror_t (action)(void *)) {
	for(int i = 0; i < HASHTABLE_SIZE; ++i)
		CHKERROR(linkedList_foreach(self->table[i], action))
	return ERR_SUCCESS;
}

