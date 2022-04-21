#include "schemeObject_t.h"
#include "machine.h"
#include "parse.h"
#include "io.h"

error_t
machine_new(machine_t * out)
{
	environment_new_global(&(out->env));
	return ERR_SUCCESS;
}

error_t
machine_eval(machine_t * self, environment_t * env, schemeObject_t ** out, schemeObject_t * obj)
{
	CHKERROR(gc_ref(&(obj->gcInfo)))
	switch (obj->kind) {
	case SCHEME_OBJECT_SYMBOL:
		if (!environment_getObject(env, out, &(obj->value.symValue))) {
			fprintf(stderr, "[ERROR] machine_eval: Not found symbol: ");
			string_writeLine(stdout, &(obj->value.symValue));
			CHKERROR(gc_deref_schemeObject(obj))
			return ERR_EVAL_NOT_FOUND_SYMBOL;
		}
		CHKERROR(gc_deref_schemeObject(obj))
		return ERR_SUCCESS;
	case SCHEME_OBJECT_CONS: {
		schemeObject_t * car;
		schemeObject_t * cdr;
		schemeObject_t * func;
		CHKERROR(schemeObject_car(obj, &car))
		if (car->kind != SCHEME_OBJECT_SYMBOL) {
			errorOut("ERROR", "machine_eval", "Not Symbol object is not applicative.");
			CHKERROR(gc_deref_schemeObject(car))
			CHKERROR(gc_deref_schemeObject(obj))
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		if (!environment_getObject(env, &func, &(car->value.symValue)) || func->kind != SCHEME_OBJECT_EXTERN_FUNCTION) {
			fprintf(stderr, "[ERROR] machine_eval: Not found function: ");
			string_writeLine(stderr, &(car->value.symValue));
			CHKERROR(gc_deref_schemeObject(car))
			CHKERROR(gc_deref_schemeObject(obj))
			return ERR_EVAL_NOT_FOUND_SYMBOL;
		}
		CHKERROR(schemeObject_cdr(obj, &cdr))
		CHKERROR(gc_deref_schemeObject(obj))
		error_t ret = func->value.extFuncValue.func(self, env, cdr, out);
		CHKERROR(gc_deref_schemeObject(func))
		CHKERROR(gc_deref_schemeObject(car))
		CHKERROR(gc_deref_schemeObject(cdr))
		return ret;
	}
		
	default:
		*out = obj;
		return ERR_SUCCESS;
	}
}

error_t
machine_lambda(machine_t * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out) {
	if(!schemeObject_isList(val)) {
		errorOut("ERROR", "lambda", "argument must be list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(gc_ref(&(val->gcInfo)))
	*out = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if(*out == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(gc_ref(&(env->gcInfo)))
	CHKERROR(gc_ref(&(val->gcInfo)))
	CHKERROR(schemeObject_new_procedure(*out, env, val))
	CHKERROR(gc_deref_schemeObject(val))
	CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}
