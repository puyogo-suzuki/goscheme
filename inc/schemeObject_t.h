#pragma once
#include <stdint.h>
#include <string_t.h>
//#include "machine.h" <- cyclic reference!
//#include "environment.h" <- cyclic reference!
#include "error.h"
#include "gc.h"

struct environment;
struct machine;

#define SCHEME_OBJECT_NILL NULL


typedef enum schemeObject_kind {
	SCHEME_OBJECT_SYMBOL,
	SCHEME_OBJECT_STRING,
	SCHEME_OBJECT_NUMBER,
	SCHEME_OBJECT_CONS,
	SCHEME_OBJECT_EXTERN_FUNCTION,
	SCHEME_OBJECT_PROCEDURE
} schemeObject_kind_t;

typedef struct schemeObject {
	schemeObject_kind_t kind;
	gcInfo_t gcInfo;
	union {
		string_t strValue;
		int32_t numValue;
		string_t symValue;
		struct {
			struct schemeObject * next;
			struct schemeObject * value;
		} consValue;
		struct {
			struct environment * environment; // closure
			error_t (*func)(struct machine *, struct environment * env, struct schemeObject *, struct schemeObject **);
		} extFuncValue;
		struct {
			struct environment * environment;
			struct schemeObject * body;
		} procedureValue;
	} value;
} schemeObject_t;

#define schemeObject_new_cons2(out, value) schemeObject_new_cons(out, value, SCHEME_OBJECT_NILL)

error_t 
schemeObject_new_string(schemeObject_t * out, string_t str);
error_t 
schemeObject_new_number(schemeObject_t * out, int32_t num);
error_t 
schemeObject_new_symbol(schemeObject_t * out, string_t sym);
error_t 
schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next);
error_t
schemeObject_new_extFunc(schemeObject_t * out, struct environment * environment, error_t (*func)(struct machine *, struct environment *, schemeObject_t *, schemeObject_t **));
error_t
schemeObject_new_procedure(schemeObject_t * out, struct environment * environment, schemeObject_t * body);

error_t
schemeObject_copy_onedepth(schemeObject_t ** out, schemeObject_t * inobj);
error_t
schemeObject_copy(schemeObject_t ** out, schemeObject_t * inobj);

bool
schemeObject_isList(schemeObject_t * self);
bool
schemeObject_isListLimited(schemeObject_t * self, int32_t listLength);

error_t schemeObject_cons(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out);
error_t schemeObject_car(schemeObject_t * self, schemeObject_t ** out);
error_t schemeObject_car2(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out);
error_t schemeObject_cdr(schemeObject_t * self, schemeObject_t ** out);
error_t schemeObject_cdr2(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out);

error_t schemeObject_quote(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out);

error_t schemeObject_toString(string_t * out, schemeObject_t * inobj);