#include "schemeObject_t.h"
#include "machine.h"
#include "parse.h"
#include "io.h"
#include "common.h"

error_t
machine_new(machine_t * out)
{
	environment_new_global(&(out->env));
	return ERR_SUCCESS;
}

error_t
machine_begin(machine_t * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out) {
	schemeObject_t * current = val;
	schemeObject_t * ret = SCHEME_OBJECT_NILL;
	if(!schemeObject_isList(val)) {
		errorOut("ERROR", "begin", "begin requires list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	while(current != SCHEME_OBJECT_NILL) {
		if(current->kind != SCHEME_OBJECT_CONS) {
			errorOut("BUG", "begin", "Not-cons cell is coming.");
			DEBUGBREAK;
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		if(ret != SCHEME_OBJECT_NILL) gc_deref_schemeObject(ret);
		CHKERROR(machine_eval(self, env, current->value.consValue.value, &ret))
		current = current->value.consValue.next;
	}
	*out = ret;
	return ERR_SUCCESS;
}

error_t
machine_lambdaexec(machine_t * self, environment_t * env, schemeObject_t ** out, schemeObject_t * body, schemeObject_t * arg) {
	schemeObject_t * car = NULL, * cdr = NULL;
	error_t ret = ERR_SUCCESS;
	environment_t * the_env;
	CHKERROR(schemeObject_car(body, &car));
	CHKERROR(schemeObject_cdr(body, &cdr));
	the_env = (environment_t *)reallocarray(NULL, 1, sizeof(environment_t));
	if(the_env == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(environment_new(the_env, env))
	CHKERROR(gc_ref(&(the_env->gcInfo)))
	schemeObject_t * current_arg = arg, * current_car = car;
	while(1) {
		if(current_arg == SCHEME_OBJECT_NILL && current_car == SCHEME_OBJECT_NILL)
			goto L_SUCC;
		if(current_arg == SCHEME_OBJECT_NILL || current_car == SCHEME_OBJECT_NILL) {
			errorOut("ERROR", "machine_lambdaexec", "Count of Argument mismatch");
			ret  = ERR_EVAL_ARGUMENT_MISMATCH;
			goto L_FAIL;
		}
		if(current_car->kind == SCHEME_OBJECT_SYMBOL) {
			schemeObject_t * res;
			string_t n;
			CHKERROR(schemeObject_map(self, the_env, &res, current_arg, machine_eval))  // res.rc++
			CHKERROR(string_copy(&n, &(current_car->value.symValue)))
			CHKERROR(environment_register(the_env, n, res))
			goto L_SUCC;
		}
		if(current_arg->kind != SCHEME_OBJECT_CONS || current_car->kind != SCHEME_OBJECT_CONS || current_car->value.consValue.value->kind != SCHEME_OBJECT_SYMBOL) {
			errorOut("ERROR", "machine_lambdaexec", "Not cons cell or cell's value is not symbol.");
			ret = ERR_EVAL_INVALID_OBJECT_TYPE;
			goto L_FAIL;
		}
		CHKERROR(environment_setq2(self, the_env, &(current_car->value.consValue.value->value.symValue), current_arg->value.consValue.value))
		current_arg = current_arg->value.consValue.next;
		current_car = current_car->value.consValue.next;
	}
	L_SUCC:
	CHKERROR(machine_begin(self, the_env, cdr, out))
	L_FAIL:
	if(car != SCHEME_OBJECT_NILL) CHKERROR(gc_deref_schemeObject(car))
	CHKERROR(gc_deref_schemeObject(cdr))
	CHKERROR(gc_deref_environment(the_env))
	return ret;
}

error_t
machine_eval(machine_t * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out)
{
	CHKERROR(gc_ref(&(val->gcInfo)))
	switch (val->kind) {
	case SCHEME_OBJECT_SYMBOL:
		if (!environment_getObject(env, out, &(val->value.symValue))) {
			fprintf(stderr, "[ERROR] machine_eval: Not found symbol: ");
			string_writeLine(stdout, &(val->value.symValue));
			CHKERROR(gc_deref_schemeObject(val))
			return ERR_EVAL_NOT_FOUND_SYMBOL;
		}
		CHKERROR(gc_deref_schemeObject(val))
		return ERR_SUCCESS;
	case SCHEME_OBJECT_CONS: {
		schemeObject_t * car, * cdr, * func;
		CHKERROR(schemeObject_car(val, &car))
		CHKERROR(machine_eval(self, env, car, &func))
		CHKERROR(schemeObject_cdr(val, &cdr))
		CHKERROR(gc_deref_schemeObject(val))
		error_t ret;
		switch(func->kind) {
			case SCHEME_OBJECT_EXTERN_FUNCTION:
				ret = func->value.extFuncValue.func(self, env, cdr, out);
				break;
			case SCHEME_OBJECT_PROCEDURE:
				ret = machine_lambdaexec(self, func->value.procedureValue.environment, out, func->value.procedureValue.body, cdr);
				break;
			default: {
				string_t errstr;
				fprintf(stderr, "[ERROR] machine_eval: Not function: ");
				schemeObject_toString(&errstr, car);
				string_writeLine(stderr, &errstr);
				string_free(&errstr);
				ret =  ERR_EVAL_NOT_FOUND_SYMBOL;
			}
		}
		CHKERROR(gc_deref_schemeObject(func))
		CHKERROR(gc_deref_schemeObject(car))
		if(cdr != SCHEME_OBJECT_NILL) CHKERROR(gc_deref_schemeObject(cdr))
		return ret;
	}
		
	default:
		*out = val;
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
