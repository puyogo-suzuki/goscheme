#include <stdint.h>
#include <string_t.h>
#include "error.h"

#define SCHEME_OBJECT_NILL NULL

typedef enum schemeObject_kind {
	SCHEME_OBJECT_SYMBOL,
	SCHEME_OBJECT_STRING,
	SCHEME_OBJECT_NUMBER,
	SCHEME_OBJECT_CONS
} schemeObject_kind_t;

typedef struct schemeObject {
	schemeObject_kind_t kind;
	union {
		string_t strValue;
		int32_t numValue;
		string_t symValue;
		struct {
			struct schemeObject * next;
			struct schemeObject * value;
		} consValue;
	} value;
} schemeObject_t;

#define schemeObject_new_cons2(out, value) schemeObject_new_cons(out, value, SCHEME_OBJECT_NILL)

error_t schemeObject_new_string(schemeObject_t * out, string_t str);
error_t schemeObject_new_number(schemeObject_t * out, int32_t num);
error_t schemeObject_new_symbol(schemeObject_t * out, string_t sym);
error_t schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next);
error_t schemeObject_toString(string_t * out, schemeObject_t * inobj);