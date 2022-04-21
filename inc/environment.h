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

error_t
environment_new(environment_t * out, environment_t * parent);

error_t
environment_new_global(environment_t * out);

error_t
environment_clone(environment_t * out, environment_t * inenv);

bool
environment_getObject(environment_t * self, schemeObject_t ** outValue, string_t * str);

error_t
environment_register(environment_t * self, string_t name, schemeObject_t * val);

error_t
environment_setq2(struct machine * self, environment_t * env, string_t * name, schemeObject_t * val);
error_t
environment_setq(struct machine * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out);
error_t
environment_set_destructive(struct machine * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out);