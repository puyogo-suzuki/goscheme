#pragma once
#include "hashtable.h"
#include "schemeObject_t.h"
#include "gc.h"

struct machine;

typedef struct environment {
	struct environment * parent;
	gcInfo_t gcInfo;
	hashtable_t env;
} environment_t;

gserror_t
environment_new(environment_t * out, environment_t * parent);

gserror_t
environment_new_global(environment_t * out);

gserror_t
environment_free(environment_t * self);

gserror_t
environment_clone(environment_t * out, environment_t * inenv);

bool
environment_getObject(environment_t * self, schemeObject_t ** outValue, string_t * str);

gserror_t
environment_register(environment_t * self, string_t name, schemeObject_t * val);

gserror_t
environment_setq3(struct machine * self, environment_t * env, string_t * name, schemeObject_t * val);
gserror_t
environment_setq2(struct machine * self, environment_t * env, environment_t * defineto, string_t * name, schemeObject_t * val);
DECL_SCHEMEFUNC(environment_let);
DECL_SCHEMEFUNC(environment_letaster);
DECL_SCHEMEFUNC(environment_setq);
DECL_SCHEMEFUNC(environment_set_destructive);
DECL_SCHEMEFUNC(environment_define_macro);