#pragma once
#include "environment.h"
#include "greenthread.h"
#if _MSC_VER
#include <windows.h>
#endif

typedef struct machine {
	environment_t * env;
	runner_t * runner;
#if _MSC_VER
	LPVOID * fiber;
#endif
} machine_t;

typedef struct evaluationResult{
	enum {EVALUATIONRESULT_EVALUATED, EVALUATIONRESULT_TAILCALL} kind;
	union {
		schemeObject_t * evaluatedValue;
		struct {
			schemeObject_t * lambdaValue;
            schemeObject_t * arguments;
		} tailcallValue;
	} value;
} evaluationResult_t;

gserror_t
machine_new(machine_t * out, environment_t * env);

gserror_t
machine_makeforce(machine_t * self, evaluationResult_t inresult, schemeObject_t ** out);

DECL_SCHEMEFUNC(machine_begin);
DECL_SCHEMEFUNC_FORCE(machine_evalforce);
DECL_SCHEMEFUNC(machine_eval);
DECL_SCHEMEFUNC(machine_lambda);
DECL_SCHEMEFUNC_FORCE(machine_macro);
