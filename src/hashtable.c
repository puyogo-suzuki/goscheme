#include <hashtable.h>

error_t
hashtable_new(hashtable_t * out) {
	for (int i = 0; i < HASHTABLE_SIZE; ++i) {
		out->table[i] = NULL;
	}
	return ERR_SUCCESS;
}

error_t
hashtable_add(hashtable_t * out, void * value, size_t valueSize, int32_t(hasher(void *))) {
	return linkedList_add(&(out->table[hasher(value) % HASHTABLE_SIZE]), value, valueSize);
}

bool
hashtable_get(hashtable_t * self, void ** outValue, void * value, int32_t(hasher(void *)), bool(comparer(void *, void *))) {
	return linkedList_search(self->table[hasher(value) % HASHTABLE_SIZE], outValue, value, comparer);
}