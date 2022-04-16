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
	switch (obj->kind) {
	case SCHEME_OBJECT_SYMBOL:
		if (!environment_getObject(env, out, &(obj->value.symValue))) {
			printf("Not found symbol: ");
			string_writeLine(stdout, &(obj->value.symValue));
			return ERR_EVAL_NOT_FOUND_SYMBOL;
		}
		return ERR_SUCCESS;
	case SCHEME_OBJECT_CONS: {
		schemeObject_t * car;
		schemeObject_t * cdr;
		schemeObject_t * func;
		CHKERROR(schemeObject_car(obj, &car))
		CHKERROR(schemeObject_cdr(obj, &cdr))
		if (car->kind != SCHEME_OBJECT_SYMBOL) {
			printf("No-Symbol is not applicative.\n");
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		if (!environment_getObject(env, &func, &(car->value.symValue)) || func->kind != SCHEME_OBJECT_EXTERN_FUNCTION) {
			printf("Not found function: ");
			string_writeLine(stdout, &(car->value.symValue));
			return ERR_EVAL_NOT_FOUND_SYMBOL;
		}
		return func->value.extFuncValue.func(self, env, cdr, out);
	}
		
	default:
		*out = obj;
		return ERR_SUCCESS;
	}
}
