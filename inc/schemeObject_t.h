#pragma once
#include <stdint.h>
#include "string_t.h"
//#include "machine.h" <- cyclic reference!
//#include "environment.h" <- cyclic reference!
#include "error.h"
#include "gc.h"

struct environment;
struct machine;
struct evaluationResult;

#define SCHEME_OBJECT_NILL NULL
// current machine, current environment, input arguments, output.
typedef gserror_t schemeFunction_t(struct machine *, struct environment * env, struct schemeObject *, struct evaluationResult *);

#define DECL_SCHEMEFUNC(name) gserror_t name(struct machine * self, struct environment * env, schemeObject_t * val, struct evaluationResult * out)
#define DECL_SCHEMEFUNC_FORCE(name) gserror_t name(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out)
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
			schemeFunction_t * func;
		} extFuncValue;
		struct {
			struct environment * environment;
			struct schemeObject * body;
		} procedureValue;
	} value;
} schemeObject_t;

#define schemeObject_new_cons2(out, value) schemeObject_new_cons(out, value, SCHEME_OBJECT_NILL)

gserror_t 
schemeObject_new_string(schemeObject_t * out, string_t str);
gserror_t 
schemeObject_new_number(schemeObject_t * out, int32_t num);
gserror_t 
schemeObject_new_symbol(schemeObject_t * out, string_t sym);
gserror_t 
schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next);
gserror_t
schemeObject_new_extFunc(schemeObject_t * out, struct environment * environment, schemeFunction_t * func);
gserror_t
schemeObject_new_procedure(schemeObject_t * out, struct environment * environment, schemeObject_t * body);

gserror_t
schemeObject_copy_onedepth(schemeObject_t ** out, schemeObject_t * inobj);
gserror_t
schemeObject_copy(schemeObject_t ** out, schemeObject_t * inobj);

bool
schemeObject_isList(schemeObject_t * self);
int32_t
schemeObject_length(schemeObject_t * self);

gserror_t schemeObject_car(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_cdr(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_cadr(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_cddr(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_caddr(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_cdddr(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_cadddr(schemeObject_t * self, schemeObject_t ** out);
gserror_t schemeObject_cddddr(schemeObject_t * self, schemeObject_t ** out);

bool
schemeObject_eqp(schemeObject_t * lhs, schemeObject_t * rhs);
bool
schemeObject_equalp(schemeObject_t * lhs, schemeObject_t * rhs);

gserror_t
schemeObject_map(struct machine * self, struct environment * env, schemeObject_t ** out, schemeObject_t * inobj, schemeFunction_t * mapper);

DECL_SCHEMEFUNC(schemeObject_quote);

gserror_t schemeObject_toString(string_t * out, schemeObject_t * inobj);