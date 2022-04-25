#pragma once
#include "environment.h"

typedef struct machine {
	environment_t env;
} machine_t;

error_t
machine_new(machine_t * out);

DECL_SCHEMEFUNC(machine_begin);
DECL_SCHEMEFUNC(machine_eval);
DECL_SCHEMEFUNC(machine_lambda);
