#include <stdint.h>
#include <string_t.h>

#define SCHEME_OBJECT_CONS_TERMINATOR NULL

typedef enum schemeObject_kind {
	SCHEME_OBJECT_SYMBOL,
	SCHEME_OBJECT_STRING,
	SCHEME_OBJECT_NUMBER,
	SCHEME_OBJECT_NIL,
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

#define schemeObject_new_cons2(out, value) schemeObject_new_cons(out, value, SCHEME_OBJECT_CONS_TERMINATOR)

bool schemeObject_new_nil(schemeObject_t * out);
bool schemeObject_new_string(schemeObject_t * out, string_t str);
bool schemeObject_new_number(schemeObject_t * out, int32_t num);
bool schemeObject_new_symbol(schemeObject_t * out, string_t sym);
bool schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next);