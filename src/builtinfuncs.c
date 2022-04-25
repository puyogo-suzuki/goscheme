#include "builtinfuncs.h"
#include "schemeObject_t.h"
#include "gc.h"
#include "machine.h"
#include "common.h"
#include "schemeObject_predefined_object.h"

#define ONE_ARGUMENT_FUNC(funcname, funcname_str, body)  error_t \
funcname(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) { \
	schemeObject_t * outobj;  \
	if (!schemeObject_isListLimited(val, 1)) { \
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

error_t
builtin_if(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	if (!schemeObject_isListLimited(val, 3)) {
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

error_t
builtin_cons(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	schemeObject_t * outobj;
	if (!schemeObject_isListLimited(val, 2)) {
		errorOut("ERROR", "cons", "cons requires 2 arguments.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	schemeObject_t * carres = NULL, * cdrres = NULL, * cadrres = NULL, * carevalres = NULL, * cadrevalres = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	CHKERROR(schemeObject_car(val, &carres))
	CHKERROR(schemeObject_cdr(val, &cdrres))
	CHKERROR(gc_deref_schemeObject(val))
	CHKERROR(schemeObject_car(cdrres, &cadrres))
	CHKERROR(gc_deref_schemeObject(cdrres))
	CHKERROR(machine_evalforce(self, env, carres, &carevalres))
	CHKERROR(gc_deref_schemeObject(carres))
	CHKERROR(machine_evalforce(self, env, cadrres, &cadrevalres))
	CHKERROR(gc_deref_schemeObject(cadrres))
	outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if(outobj == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(schemeObject_new_cons(outobj, carevalres, cadrevalres))
	// carevalres, cadrevalresはconsでref incrementしないので，ref decrementしない．
	// *out が参照しているので，それぞれ1ずつあり，うまくいくはずである．
	CHKERROR(gc_ref(&(outobj->gcInfo)))
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = outobj;
	return ERR_SUCCESS;
}

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

error_t
builtin_additive(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	int32_t retval = 0;
    if(!schemeObject_isList(val)) {
		errorOut("ERROR", "+", "proper list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo)))
    schemeObject_t * car, * cdr = val, * carres;
    while(cdr != SCHEME_OBJECT_NILL) {
        READ_HEAD(self, env, car, carres, cdr)
        if(carres->kind != SCHEME_OBJECT_NUMBER) {
            errorOut("ERROR", "+", "Not number coming.");
            CHKERROR(gc_deref_schemeObject(cdr))
            CHKERROR(gc_deref_schemeObject(carres))
            return ERR_EVAL_INVALID_OBJECT_TYPE;
        }
        retval += carres->value.numValue;
        CHKERROR(gc_deref_schemeObject(carres))
    }
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
    if(outobj == NULL) return ERR_OUT_OF_MEMORY;
    CHKERROR(schemeObject_new_number(outobj, retval))
    CHKERROR(gc_ref(&(outobj->gcInfo)))
    out->kind = EVALUATIONRESULT_EVALUATED;
    out->value.evaluatedValue = outobj;
    return ERR_SUCCESS;
}

error_t
builtin_subtract(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	int32_t retval = 0;
    if(!schemeObject_isList(val)) {
		errorOut("ERROR", "-", "proper list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo)))
    schemeObject_t * car, * cdr = val, * carres;
    READ_HEAD(self, env, car, carres, cdr)
    if(carres->kind != SCHEME_OBJECT_NUMBER) goto L_NOT_COMING_NUMBER;
    retval = cdr == SCHEME_OBJECT_NILL ? -carres->value.numValue : carres->value.numValue;
    while(cdr != SCHEME_OBJECT_NILL) {
        READ_HEAD(self, env, car, carres, cdr)
        if(carres->kind != SCHEME_OBJECT_NUMBER) goto L_NOT_COMING_NUMBER;
        retval -= carres->value.numValue;
        CHKERROR(gc_deref_schemeObject(carres))
    }
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
    if(outobj == NULL) return ERR_OUT_OF_MEMORY;
    CHKERROR(schemeObject_new_number(outobj, retval))
    CHKERROR(gc_ref(&(outobj->gcInfo)))
    out->kind = EVALUATIONRESULT_EVALUATED;
    out->value.evaluatedValue = outobj;
    return ERR_SUCCESS;

L_NOT_COMING_NUMBER:
    errorOut("ERROR", "-", "Not number coming.");
    CHKERROR(gc_deref_schemeObject(cdr))
    CHKERROR(gc_deref_schemeObject(carres))
    return ERR_EVAL_INVALID_OBJECT_TYPE;
}

error_t
builtin_multiplication(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	int32_t retval = 1;
    if(!schemeObject_isList(val)) {
		errorOut("ERROR", "*", "proper list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo)))
    schemeObject_t * car, * cdr = val, * carres;
    while(cdr != SCHEME_OBJECT_NILL) {
        READ_HEAD(self, env, car, carres, cdr)
        if(carres->kind != SCHEME_OBJECT_NUMBER) {
            errorOut("ERROR", "*", "Not number coming.");
            CHKERROR(gc_deref_schemeObject(cdr))
            CHKERROR(gc_deref_schemeObject(carres))
            return ERR_EVAL_INVALID_OBJECT_TYPE;
        }
        retval *= carres->value.numValue;
        CHKERROR(gc_deref_schemeObject(carres))
    }
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
    if(outobj == NULL) return ERR_OUT_OF_MEMORY;
    CHKERROR(schemeObject_new_number(outobj, retval))
    CHKERROR(gc_ref(&(outobj->gcInfo)))
    out->kind = EVALUATIONRESULT_EVALUATED;
    out->value.evaluatedValue = outobj;
    return ERR_SUCCESS;
}

error_t
builtin_division(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	int32_t retval = 0;
    if(!schemeObject_isList(val)) {
		errorOut("ERROR", "-", "proper list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo)))
    schemeObject_t * car, * cdr = val, * carres;
    READ_HEAD(self, env, car, carres, cdr)
    if(car->kind != SCHEME_OBJECT_NUMBER) goto L_NOT_COMING_NUMBER;
    retval = cdr == SCHEME_OBJECT_NILL ? 0 : carres->value.numValue; // TODO: Rational Value.
    while(cdr != SCHEME_OBJECT_NILL) {
        READ_HEAD(self, env, car, carres, cdr)
        if(carres->kind != SCHEME_OBJECT_NUMBER) goto L_NOT_COMING_NUMBER;
        retval /= carres->value.numValue;
        CHKERROR(gc_deref_schemeObject(carres))
    }
    schemeObject_t * outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
    if(outobj == NULL) return ERR_OUT_OF_MEMORY;
    CHKERROR(schemeObject_new_number(outobj, retval))
    CHKERROR(gc_ref(&(outobj->gcInfo)))
    out->kind = EVALUATIONRESULT_EVALUATED;
    out->value.evaluatedValue = outobj;
    return ERR_SUCCESS;

L_NOT_COMING_NUMBER:
    errorOut("ERROR", "-", "Not number coming.");
    CHKERROR(gc_deref_schemeObject(cdr))
    CHKERROR(gc_deref_schemeObject(carres))
    return ERR_EVAL_INVALID_OBJECT_TYPE;
}

#define PRED_FUNC(funcname, funcname_str, pred)  ONE_ARGUMENT_FUNC(funcname, funcname_str, { \
    outobj = (pred) ? &predefined_t : &predefined_f; \
    CHKERROR(gc_deref_schemeObject(arg0)) \
    CHKERROR(gc_ref(&(outobj->gcInfo))) \
})

PRED_FUNC(builtin_numberp, "number?", arg0->kind == SCHEME_OBJECT_NUMBER)
PRED_FUNC(builtin_stringp, "string?", arg0->kind == SCHEME_OBJECT_STRING)
PRED_FUNC(builtin_nullp, "null?", arg0 == SCHEME_OBJECT_NILL)
PRED_FUNC(builtin_symbolp, "symbol?", arg0->kind == SCHEME_OBJECT_SYMBOL)
PRED_FUNC(builtin_procedurep, "procedure?", arg0->kind == SCHEME_OBJECT_PROCEDURE || arg0->kind == SCHEME_OBJECT_EXTERN_FUNCTION)

