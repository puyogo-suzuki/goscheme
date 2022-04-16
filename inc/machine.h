#pragma once
#include "environment.h"

typedef struct machine {
	environment_t env;
} machine_t;

error_t
machine_new(machine_t * out);

error_t
machine_eval(machine_t * self, environment_t * env, schemeObject_t ** out, schemeObject_t * obj);
