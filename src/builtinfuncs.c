#include "builtinfuncs.h"
#include "schemeObject_t.h"
#include "gc.h"
#include "machine.h"
#include "common.h"
#include "schemeObject_predefined_object.h"

#define ONE_ARGUMENT_FUNC(funcname, funcname_str, body)  gserror_t \
funcname(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	schemeObject_t * outobj;  \
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
} \

#define TWO_ARGUMENT_FUNC(funcname, funcname_str, body, refdec)  gserror_t \
funcname(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	schemeObject_t * outobj;  \
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
} \


gserror_t
builtin_if(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	if (schemeObject_length(val) != 3) {
		errorOut("ERROR", "if", "if requires 3 argument.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	schemeObject_t * carres = NULL, * arg0 = NULL, * cdr = NULL, * cadr = NULL, * cddr = NULL,  * caddr  = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	CHKERROR(schemeObject_car(val, &carres))
	CHKERROR(gc_deref_schemeObject(val))
	CHKERROR(machine_evalforce(self, env, carres, &arg0))
	CHKERROR(gc_deref_schemeObject(carres))

    CHKERROR(schemeObject_cdr(val, &cdr))
    CHKERROR(schemeObject_car(cdr, &cadr)) // then
    CHKERROR(schemeObject_cdr(cdr, &cddr))
	CHKERROR(gc_deref_schemeObject(cdr))
    CHKERROR(schemeObject_car(cddr, &caddr)) // else
	CHKERROR(gc_deref_schemeObject(cddr))
    
    CHKERROR(machine_eval(self, env, arg0 != &predefined_f ? cadr : caddr, out))
    
	CHKERROR(gc_deref_schemeObject(cadr))
	CHKERROR(gc_deref_schemeObject(caddr))

	CHKERROR(gc_deref_schemeObject(arg0))
	return ERR_SUCCESS;
}

TWO_ARGUMENT_FUNC(builtin_cons, "cons", \
	outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t)); \
	if(outobj == NULL) return ERR_OUT_OF_MEMORY; \
    CHKERROR(schemeObject_new_cons(outobj, arg0, arg1)) \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
, false)


ONE_ARGUMENT_FUNC(builtin_car, "car", CHKERROR(schemeObject_car(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_cdr, "cdr", CHKERROR(schemeObject_cdr(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_cadr, "cadr", CHKERROR(schemeObject_cadr(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_cddr, "cddr", CHKERROR(schemeObject_cddr(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_caddr, "caddr", CHKERROR(schemeObject_caddr(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_cdddr, "cdddr", CHKERROR(schemeObject_cdddr(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_cadddr, "cadddr", CHKERROR(schemeObject_cadddr(arg0, &outobj)))
ONE_ARGUMENT_FUNC(builtin_cddddr, "cddddr", CHKERROR(schemeObject_cddddr(arg0, &outobj)))

#define READ_HEAD(self, env, car, carres, cdr)  { \
    schemeObject_t * prev_cdr = cdr; \
    CHKERROR(schemeObject_car(prev_cdr, &car)) \
    CHKERROR(machine_evalforce(self, env, car, &carres)) \
    CHKERROR(gc_deref_schemeObject(car)) \
    CHKERROR(schemeObject_cdr(prev_cdr, &cdr)) \
    CHKERROR(gc_deref_schemeObject(prev_cdr)) \
}

#define ARITHMETIC_LEAST0_FUNC(name, name_str, retval_init, retval_final, updater) gserror_t \
name(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	retval_init \
    if(!schemeObject_isList(val)) { \
		errorOut("ERROR", name_str, "proper list."); \
		return ERR_EVAL_INVALID_OBJECT_TYPE; \
    } \
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo))) \
    schemeObject_t * car, * cdr = val, * carres; \
    while(cdr != SCHEME_OBJECT_NILL) { \
        READ_HEAD(self, env, car, carres, cdr) \
        if(carres->kind != SCHEME_OBJECT_NUMBER) { \
            errorOut("ERROR", name_str, "Not number coming."); \
            CHKERROR(gc_deref_schemeObject(cdr)) \
            CHKERROR(gc_deref_schemeObject(carres)) \
            return ERR_EVAL_INVALID_OBJECT_TYPE; \
        } \
        updater \
        CHKERROR(gc_deref_schemeObject(carres)) \
    } \
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t)); \
    if(outobj == NULL) return ERR_OUT_OF_MEMORY; \
    retval_final \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
    out->kind = EVALUATIONRESULT_EVALUATED; \
    out->value.evaluatedValue = outobj; \
    return ERR_SUCCESS; \
} \

ARITHMETIC_LEAST0_FUNC(builtin_additive, "+", int32_t retval = 0; , CHKERROR(schemeObject_new_number(outobj, retval)), retval += carres->value.numValue;)
ARITHMETIC_LEAST0_FUNC(builtin_multiplication, "*", int32_t retval = 1; , CHKERROR(schemeObject_new_number(outobj, retval)), retval *= carres->value.numValue;)

#define BOOLEAN_LEAST0_FUNC(name, name_str, retval_init, retval_final, updater) gserror_t \
name(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	retval_init \
    if(!schemeObject_isList(val)) { \
		errorOut("ERROR", name_str, "proper list."); \
		return ERR_EVAL_INVALID_OBJECT_TYPE; \
    } \
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo))) \
    schemeObject_t * car, * cdr = val, * carres; \
    while(cdr != SCHEME_OBJECT_NILL) { \
        READ_HEAD(self, env, car, carres, cdr) \
        updater \
        CHKERROR(gc_deref_schemeObject(carres)) \
    } \
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t)); \
    if(outobj == NULL) return ERR_OUT_OF_MEMORY; \
    retval_final \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
    out->kind = EVALUATIONRESULT_EVALUATED; \
    out->value.evaluatedValue = outobj; \
    return ERR_SUCCESS; \
} \

BOOLEAN_LEAST0_FUNC(builtin_and, "and", bool retval = true; schemeObject_t * lastValue = &predefined_t; CHKERROR(gc_ref(&(lastValue->gcInfo))), outobj = retval ? lastValue : &predefined_f; CHKERROR(gc_deref_schemeObject(lastValue)); , retval &= (carres != &predefined_f); CHKERROR(gc_deref_schemeObject(lastValue)); lastValue = carres; CHKERROR(gc_ref(&(lastValue->gcInfo))))
BOOLEAN_LEAST0_FUNC(builtin_or, "or", bool retval = true; schemeObject_t * lastValue = &predefined_f; CHKERROR(gc_ref(&(lastValue->gcInfo))), outobj = retval ? lastValue : &predefined_f; CHKERROR(gc_deref_schemeObject(lastValue));, retval |= (carres != &predefined_f); if (carres != &predefined_f) { CHKERROR(gc_deref_schemeObject(lastValue)); lastValue = carres; CHKERROR(gc_ref(&(lastValue->gcInfo))) })

#define ARITHMETIC_LEAST1_FUNC(name, name_str, arglength_pred, retval_init, init2, retval_final, updater) gserror_t \
name(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	retval_init \
	int32_t arglen = schemeObject_length(val); \
    if(arglength_pred) { \
		errorOut("ERROR", name_str, "proper list."); \
		return ERR_EVAL_INVALID_OBJECT_TYPE; \
    } \
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo))) \
    schemeObject_t * car, * cdr = val, * carres; \
    READ_HEAD(self, env, car, carres, cdr) \
    if(carres->kind != SCHEME_OBJECT_NUMBER) goto L_NOT_COMING_NUMBER; \
    init2 \
    while(cdr != SCHEME_OBJECT_NILL) { \
        READ_HEAD(self, env, car, carres, cdr) \
        if(carres->kind != SCHEME_OBJECT_NUMBER) goto L_NOT_COMING_NUMBER; \
        retval -= carres->value.numValue; \
        CHKERROR(gc_deref_schemeObject(carres)) \
    } \
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t)); \
    if(outobj == NULL) return ERR_OUT_OF_MEMORY; \
    retval_final \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
    out->kind = EVALUATIONRESULT_EVALUATED; \
    out->value.evaluatedValue = outobj; \
    return ERR_SUCCESS; \
\
L_NOT_COMING_NUMBER: \
    errorOut("ERROR", name_str, "Not number coming."); \
    CHKERROR(gc_deref_schemeObject(cdr)) \
    CHKERROR(gc_deref_schemeObject(carres)) \
    return ERR_EVAL_INVALID_OBJECT_TYPE; \
} \

ARITHMETIC_LEAST1_FUNC(builtin_subtract, "-", arglen < 1, int32_t retval = 0; , retval = cdr == SCHEME_OBJECT_NILL ? -carres->value.numValue : carres->value.numValue; , CHKERROR(schemeObject_new_number(outobj, retval)), retval -= carres->value.numValue;)
ARITHMETIC_LEAST1_FUNC(builtin_division, "/", arglen < 1, int32_t retval = 0;, retval = cdr == SCHEME_OBJECT_NILL ? 0 : carres->value.numValue;, CHKERROR(schemeObject_new_number(outobj, retval)), retval /= carres->value.numValue;)
ARITHMETIC_LEAST1_FUNC(builtin_equate, "=", arglen < 2, bool retval = true; int32_t headVal = 0; , headVal = carres->value.numValue;, outobj = retval ? &predefined_t : &predefined_f; , retval &= (headVal == carres->value.numValue);)
ARITHMETIC_LEAST1_FUNC(builtin_leq, "<=", arglen < 2, bool retval = true; int32_t headVal = 0;, headVal = carres->value.numValue; , outobj = retval ? &predefined_t : &predefined_f;, retval &= (headVal <= carres->value.numValue);)
ARITHMETIC_LEAST1_FUNC(builtin_less, "<", arglen < 2, bool retval = true; int32_t headVal = 0; , headVal = carres->value.numValue;, outobj = retval ? &predefined_t : &predefined_f; , retval &= (headVal < carres->value.numValue);)
ARITHMETIC_LEAST1_FUNC(builtin_geq, ">=", arglen < 2, bool retval = true; int32_t headVal = 0;, headVal = carres->value.numValue; , outobj = retval ? &predefined_t : &predefined_f;, retval &= (headVal >= carres->value.numValue);)
ARITHMETIC_LEAST1_FUNC(builtin_greater, ">", arglen < 2, bool retval = true; int32_t headVal = 0;, headVal = carres->value.numValue; , outobj = retval ? &predefined_t : &predefined_f;, retval &= (headVal > carres->value.numValue);)

#define PRED_FUNC(funcname, funcname_str, pred)  ONE_ARGUMENT_FUNC(funcname, funcname_str, { \
    outobj = (pred) ? &predefined_t : &predefined_f; \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
})

PRED_FUNC(builtin_numberp, "number?", arg0->kind == SCHEME_OBJECT_NUMBER)
PRED_FUNC(builtin_booleanp, "boolean?", arg0 == &predefined_t || arg0 == &predefined_f)
PRED_FUNC(builtin_stringp, "string?", arg0->kind == SCHEME_OBJECT_STRING)
PRED_FUNC(builtin_nullp, "null?", arg0 == SCHEME_OBJECT_NILL)
PRED_FUNC(builtin_pairp, "pair?", arg0->kind == SCHEME_OBJECT_CONS)
PRED_FUNC(builtin_listp, "list?", schemeObject_isList(arg0))
PRED_FUNC(builtin_symbolp, "symbol?", arg0->kind == SCHEME_OBJECT_SYMBOL)
PRED_FUNC(builtin_procedurep, "procedure?", arg0->kind == SCHEME_OBJECT_PROCEDURE || arg0->kind == SCHEME_OBJECT_EXTERN_FUNCTION)

ONE_ARGUMENT_FUNC(builtin_not, "not", { \
    outobj = (arg0 == &predefined_f) ? &predefined_t : &predefined_f; \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
})
ONE_ARGUMENT_FUNC(builtin_length, "length", { \
    int32_t retVal = schemeObject_length(arg0); \
    if (retVal == -1) { \
        errorOut("ERROR", "length", "proper list."); \
        return ERR_EVAL_INVALID_OBJECT_TYPE; \
    } \
    outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t)); \
    if (outobj == NULL) return ERR_OUT_OF_MEMORY; \
    CHKERROR(schemeObject_new_number(outobj, retVal))\
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
})