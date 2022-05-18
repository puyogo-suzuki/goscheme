#include "schemeObject_t.h"
#include "machine.h"
#include "parse.h"
#include "io.h"
#include "common.h"

gserror_t
machine_new(machine_t * out, environment_t * env)
{
	if (env == NULL) {
		env = (environment_t *)reallocarray(NULL, 1, sizeof(environment_t));
		if (env == NULL) return ERR_OUT_OF_MEMORY;
		environment_new_global(env);
	}
	out->env = env;
	out->runner = NULL;
	return ERR_SUCCESS;
}

gserror_t
machine_begin(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	schemeObject_t * current = val;
	schemeObject_t * ret = SCHEME_OBJECT_NILL;
	if(!schemeObject_isList(val)) {
		errorOut("ERROR", "begin", "begin requires list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	while(current != SCHEME_OBJECT_NILL) {
		schemeObject_t * next;
		if(current->kind != SCHEME_OBJECT_CONS) {
			errorOut("BUG", "begin", "Not-cons cell is coming.");
			DEBUGBREAK;
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		if(ret != SCHEME_OBJECT_NILL) gc_deref_schemeObject(ret);
		next = current->value.consValue.next;
		if(next == SCHEME_OBJECT_NILL) {
			CHKERROR(machine_eval(self, env, current->value.consValue.value, out))
			break;
		} else
			CHKERROR(machine_evalforce(self, env, current->value.consValue.value, &ret))
		current = next;
	}
	return ERR_SUCCESS;
}

gserror_t
machine_lambdaexec(machine_t * self, environment_t * env, evaluationResult_t * out, schemeObject_t * body, schemeObject_t * arg) {
	schemeObject_t * car = NULL, * cdr = NULL;
	gserror_t ret = ERR_SUCCESS;
	environment_t * the_env;
	CHKERROR(schemeObject_car(body, &car));
	CHKERROR(schemeObject_cdr(body, &cdr));
	the_env = (environment_t *)reallocarray(NULL, 1, sizeof(environment_t));
	if(the_env == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(environment_new(the_env, env))
	CHKERROR(gc_ref(&(env->gcInfo)))
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
			string_t n;
			CHKERROR(string_copy(&n, &(current_car->value.symValue)))
			CHKERROR(environment_register(the_env, n, arg))
			goto L_SUCC;
		}
		if(current_arg->kind != SCHEME_OBJECT_CONS || current_car->kind != SCHEME_OBJECT_CONS || current_car->value.consValue.value->kind != SCHEME_OBJECT_SYMBOL) {
			errorOut("ERROR", "machine_lambdaexec", "Not cons cell or cell's value is not symbol.");
			ret = ERR_EVAL_INVALID_OBJECT_TYPE;
			goto L_FAIL;
		}
		CHKERROR(environment_setq3(self, the_env, &(current_car->value.consValue.value->value.symValue), current_arg->value.consValue.value))
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

gserror_t
machine_macroexec(machine_t * self, environment_t * env, schemeObject_t ** out, schemeObject_t * body, schemeObject_t * arg) {
	evaluationResult_t res;
	CHKERROR(machine_lambdaexec(self, env, &res, body, arg))
	CHKERROR(machine_makeforce(self, res, out))
	return ERR_SUCCESS;
}

gserror_t
machine_makeforce(machine_t * self, evaluationResult_t inresult, schemeObject_t ** out)
{
	evaluationResult_t ret = inresult, ret_prev = inresult;
	while(ret.kind != EVALUATIONRESULT_EVALUATED) {
		ret_prev = ret;
		CHKERROR(machine_lambdaexec(self, ret.value.tailcallValue.lambdaValue->value.procedureValue.environment, &ret, ret.value.tailcallValue.lambdaValue->value.procedureValue.body, ret.value.tailcallValue.arguments))
		CHKERROR(gc_deref_schemeObject(ret_prev.value.tailcallValue.lambdaValue))
		CHKERROR(gc_deref_schemeObject(ret_prev.value.tailcallValue.arguments))
	}
	*out = ret.value.evaluatedValue;
	return ERR_SUCCESS;
}

gserror_t
machine_evalforce(machine_t * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out)
{
	evaluationResult_t evalres;
	CHKERROR(machine_eval(self, env, val, &evalres))
	CHKERROR(machine_makeforce(self, evalres, out))
	return ERR_SUCCESS;
}


gserror_t
machine_eval(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out)
{
	if(val == SCHEME_OBJECT_NILL) {
		out->kind = EVALUATIONRESULT_EVALUATED;
		out->value.evaluatedValue = SCHEME_OBJECT_NILL;
		return ERR_SUCCESS;
	}
	CHKERROR(gc_ref(&(val->gcInfo)))
	out->kind = EVALUATIONRESULT_EVALUATED;
	switch (val->kind) {
	case SCHEME_OBJECT_SYMBOL:
		if (!environment_getObject(env, &(out->value.evaluatedValue), &(val->value.symValue))) {
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
		CHKERROR(machine_evalforce(self, env, car, &func))
		CHKERROR(schemeObject_cdr(val, &cdr))
		CHKERROR(gc_deref_schemeObject(val))
		gserror_t ret = ERR_SUCCESS;
		switch(func->kind) {
			case SCHEME_OBJECT_EXTERN_FUNCTION:
				ret = func->value.extFuncValue.func(self, env, cdr, out);
				CHKERROR(gc_deref_schemeObject(func))
				break;
			case SCHEME_OBJECT_PROCEDURE:
				out->kind = EVALUATIONRESULT_TAILCALL;
				out->value.tailcallValue.lambdaValue = func;
				schemeObject_t * evaluatedArg;
				CHKERROR(schemeObject_map(self, env, &evaluatedArg, cdr, machine_eval))
				out->value.tailcallValue.arguments = evaluatedArg;
				break;
			case SCHEME_OBJECT_MACRO:
				schemeObject_t * macroOut = NULL;
				CHKERROR(machine_macroexec(self, env, &macroOut, func->value.macroValue.body, cdr))
				CHKERROR(gc_deref_schemeObject(func))
				CHKERROR(machine_eval(self, env, macroOut, out))
				CHKERROR(gc_deref_schemeObject(macroOut))
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
		CHKERROR(gc_deref_schemeObject(car))
		if(cdr != SCHEME_OBJECT_NILL) CHKERROR(gc_deref_schemeObject(cdr))
		return ret;
	}
		
	default:
		out->kind = EVALUATIONRESULT_EVALUATED;
		out->value.evaluatedValue = val;
		return ERR_SUCCESS;
	}
}

gserror_t
machine_lambda(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	schemeObject_t * outobj = NULL;
	if(!schemeObject_isList(val)) {
		errorOut("ERROR", "lambda", "argument must be list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(gc_ref(&(val->gcInfo)))
	outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if(outobj == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(gc_ref(&(env->gcInfo)))
	CHKERROR(schemeObject_new_procedure(outobj, env, val))
	CHKERROR(gc_ref(&(outobj->gcInfo)))
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = outobj;
	return ERR_SUCCESS;
}

gserror_t
machine_macro(machine_t * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out) {
	schemeObject_t * outobj = NULL;
	if(!schemeObject_isList(val)) {
		errorOut("ERROR", "macro", "argument must be list.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(gc_ref(&(val->gcInfo)))
	outobj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if(outobj == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(gc_ref(&(env->gcInfo)))
	CHKERROR(schemeObject_new_macro(outobj, env, val))
	CHKERROR(gc_ref(&(outobj->gcInfo)))
	*out = outobj;
	return ERR_SUCCESS;
}