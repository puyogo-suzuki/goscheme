#pragma once
#include "schemeObject_t.h"

#define ONE_ARGUMENT_FUNC(funcname, funcname_str, body)  gserror_t \
funcname(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	schemeObject_t * outobj = NULL;  \
	if (schemeObject_length(val) != 1) { \
		errorOut("ERROR", funcname_str, funcname_str" requires 1 argument."); \
		return ERR_EVAL_INVALID_OBJECT_TYPE; \
	} \
	schemeObject_t * carres = NULL, * arg0 = NULL; \
	CHKERROR(gc_ref(&(val->gcInfo))) \
	CHKERROR(schemeObject_car(val, &carres)) \
	CHKERROR(gc_deref_schemeObject(val)) \
	CHKERROR(machine_evalforce(self, env, carres, &arg0)) \
	CHKERROR(gc_deref_schemeObject(carres)) \
	body \
	CHKERROR(gc_deref_schemeObject(arg0)) \
	out->kind = EVALUATIONRESULT_EVALUATED; \
	out->value.evaluatedValue = outobj; \
	return ERR_SUCCESS; \
}

#define TWO_ARGUMENT_FUNC(funcname, funcname_str, body, refdec)  gserror_t \
funcname(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
    schemeObject_t * outobj = NULL;  \
    if (schemeObject_length(val) != 2) { \
        errorOut("ERROR", funcname_str, funcname_str" requires 2 arguments."); \
        return ERR_EVAL_INVALID_OBJECT_TYPE; \
    } \
    schemeObject_t * carres = NULL, * arg0 = NULL, * cdrres = NULL, * cadrres = NULL, * arg1 = NULL; \
    CHKERROR(gc_ref(&(val->gcInfo))) \
    CHKERROR(schemeObject_car(val, &carres)) \
    CHKERROR(schemeObject_cdr(val, &cdrres)) \
    CHKERROR(schemeObject_car(cdrres, &cadrres)) \
    CHKERROR(gc_deref_schemeObject(val)) \
    CHKERROR(gc_deref_schemeObject(cdrres)) \
    CHKERROR(machine_evalforce(self, env, carres, &arg0)) \
    CHKERROR(machine_evalforce(self, env, cadrres, &arg1)) \
    CHKERROR(gc_deref_schemeObject(carres)) \
    CHKERROR(gc_deref_schemeObject(cadrres)) \
    body \
    if(refdec) { \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        CHKERROR(gc_deref_schemeObject(arg1)) \
    } \
    out->kind = EVALUATIONRESULT_EVALUATED; \
	out->value.evaluatedValue = outobj; \
	return ERR_SUCCESS; \
}

// // Special Format!
DECL_SCHEMEFUNC(builtin_if);
DECL_SCHEMEFUNC(builtin_cond);
DECL_SCHEMEFUNC(builtin_do);

// // Functions
DECL_SCHEMEFUNC(builtin_cons);
DECL_SCHEMEFUNC(builtin_car);
DECL_SCHEMEFUNC(builtin_cdr);
DECL_SCHEMEFUNC(builtin_cadr);
DECL_SCHEMEFUNC(builtin_cddr);
DECL_SCHEMEFUNC(builtin_caddr);
DECL_SCHEMEFUNC(builtin_cdddr);
DECL_SCHEMEFUNC(builtin_cadddr);
DECL_SCHEMEFUNC(builtin_cddddr);

DECL_SCHEMEFUNC(builtin_and);
DECL_SCHEMEFUNC(builtin_or);
DECL_SCHEMEFUNC(builtin_numberp);
DECL_SCHEMEFUNC(builtin_additive);
DECL_SCHEMEFUNC(builtin_subtract);
DECL_SCHEMEFUNC(builtin_multiplication);
DECL_SCHEMEFUNC(builtin_division);
DECL_SCHEMEFUNC(builtin_equate);
DECL_SCHEMEFUNC(builtin_less);
DECL_SCHEMEFUNC(builtin_leq);
DECL_SCHEMEFUNC(builtin_greater);
DECL_SCHEMEFUNC(builtin_geq);
DECL_SCHEMEFUNC(builtin_nullp);
DECL_SCHEMEFUNC(builtin_pairp);
DECL_SCHEMEFUNC(builtin_listp);
DECL_SCHEMEFUNC(builtin_symbolp);
DECL_SCHEMEFUNC(builtin_list);
DECL_SCHEMEFUNC(builtin_length);
DECL_SCHEMEFUNC(builtin_memq);
DECL_SCHEMEFUNC(builtin_last);
DECL_SCHEMEFUNC(builtin_append);
DECL_SCHEMEFUNC(builtin_set_car);
DECL_SCHEMEFUNC(builtin_set_cdr);
DECL_SCHEMEFUNC(builtin_booleanp);
DECL_SCHEMEFUNC(builtin_not);
DECL_SCHEMEFUNC(builtin_stringp);
DECL_SCHEMEFUNC(builtin_string_append);
DECL_SCHEMEFUNC(builtin_symbol_string);
DECL_SCHEMEFUNC(builtin_string_symbol);
DECL_SCHEMEFUNC(builtin_string_number);
DECL_SCHEMEFUNC(builtin_number_string);
DECL_SCHEMEFUNC(builtin_procedurep);
DECL_SCHEMEFUNC(builtin_eqp);
DECL_SCHEMEFUNC(builtin_neqp);
DECL_SCHEMEFUNC(builtin_equalp);
DECL_SCHEMEFUNC(builtin_display);
#if !defined(_ESP)
DECL_SCHEMEFUNC(builtin_sleep);
DECL_SCHEMEFUNC(builtin_spawn);
#endif