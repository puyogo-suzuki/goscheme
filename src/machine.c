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
