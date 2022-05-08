#include "environment.h"
#include "machine.h"
#include "string_t.h"
#include "gc.h"
#include <stdio.h>
#include "io.h"
#include "schemeObject_predefined_object.h"
#include "builtinfuncs.h"

typedef struct hashItem {
	string_t name;
	schemeObject_t * value;
} hashItem_t;

int32_t
hashing(hashItem_t * hi) {
	return string_hash(&(hi->name));
}

gserror_t
environment_new(environment_t * out, environment_t * parent) {
	out->parent = parent;
	gcInfo_new(&out->gcInfo);
	return hashtable_new(&(out->env));
}

gserror_t
copyaction(hashItem_t * inplace) {
	schemeObject_t * sobj = inplace->value;
	string_t s = inplace->name;
	CHKERROR(schemeObject_copy(&(inplace->value), sobj))
	CHKERROR(string_copy(&(inplace->name), &s))
	return ERR_SUCCESS;
}

gserror_t
environment_clone(environment_t * out, environment_t * inenv) {
	environment_new(out, inenv->parent);
	if(inenv->parent != NULL)
		CHKERROR(gc_ref(&(inenv->parent->gcInfo)))
	CHKERROR(hashtable_copy(&(out->env), &(inenv->env), sizeof(schemeObject_t)))
	CHKERROR(hashtable_foreach(&(out->env), (gserror_t (*)(void *)) copyaction));
	return ERR_SUCCESS;
}

gserror_t
addfunc(environment_t * out, char * name, size_t name_length, schemeFunction_t * func) {
	string_t str;
	string_new_deep(&str, name, name_length);
	schemeObject_t * obj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (obj == NULL) return ERR_OUT_OF_MEMORY;
	schemeObject_new_extFunc(obj, NULL, func);
	CHKERROR(gc_ref(&(obj->gcInfo)))
	environment_register(out, str, obj);
	return ERR_SUCCESS;
}

gserror_t
addsymbol(environment_t * out, char * name, size_t name_length, schemeObject_t * obj) {
	string_t str;
	string_new_deep(&str, name, name_length);
	CHKERROR(gc_ref(&(obj->gcInfo)))
	environment_register(out, str, obj);
	return ERR_SUCCESS;
}

gserror_t
environment_new_global(environment_t * out)
{
	CHKERROR(environment_new(out, NULL))
	CHKERROR(gc_ref(&(out->gcInfo)))
	// TODO:Implement buildin function.
	// TODO: Implement let* and let correctly?
	CHKERROR(addfunc(out, "let*", 4, environment_letaster))
	CHKERROR(addfunc(out, "let", 3, environment_let))
	CHKERROR(addfunc(out, "letrec*", 7, environment_letaster))
	CHKERROR(addfunc(out, "letrec", 6, environment_let))
	CHKERROR(addfunc(out, "define", 6, environment_setq))
	CHKERROR(addfunc(out, "quote", 5, schemeObject_quote))
	CHKERROR(addfunc(out, "car", 3, builtin_car))
	CHKERROR(addfunc(out, "cdr", 3, builtin_cdr))
	CHKERROR(addfunc(out, "cadr", 4, builtin_cadr))
	CHKERROR(addfunc(out, "cddr", 4, builtin_cddr))
	CHKERROR(addfunc(out, "caddr", 5, builtin_caddr))
	CHKERROR(addfunc(out, "cdddr", 5, builtin_cdddr))
	CHKERROR(addfunc(out, "cadddr", 6, builtin_cadddr))
	CHKERROR(addfunc(out, "cddddr", 6, builtin_cddddr))

	CHKERROR(addfunc(out, "cons", 4, builtin_cons))
	CHKERROR(addfunc(out, "if", 2, builtin_if))
	CHKERROR(addfunc(out, "begin", 5, machine_begin))
	CHKERROR(addfunc(out, "lambda", 6, machine_lambda))
	CHKERROR(addfunc(out, "set!", 4, environment_set_destructive))

	CHKERROR(addfunc(out, "null?", 5, builtin_nullp))
	CHKERROR(addfunc(out, "+", 1, builtin_additive))
	CHKERROR(addfunc(out, "-", 1, builtin_subtract))
	CHKERROR(addfunc(out, "*", 1, builtin_multiplication))
	CHKERROR(addfunc(out, "/", 1, builtin_division))
	CHKERROR(addfunc(out, "=", 1, builtin_equate))
	CHKERROR(addfunc(out, "<", 1, builtin_less))
	CHKERROR(addfunc(out, "<=", 2, builtin_leq))
	CHKERROR(addfunc(out, ">", 1, builtin_greater))
	CHKERROR(addfunc(out, ">=", 2, builtin_geq))
	CHKERROR(addfunc(out, "not", 3, builtin_not))
	CHKERROR(addfunc(out, "and", 3, builtin_and))
	CHKERROR(addfunc(out, "or", 2, builtin_or))
	CHKERROR(addfunc(out, "list", 4, builtin_list))
	CHKERROR(addfunc(out, "memq", 4, builtin_memq))
	CHKERROR(addfunc(out, "last", 4, builtin_last))
	CHKERROR(addfunc(out, "append", 6, builtin_append))
	CHKERROR(addfunc(out, "length", 6, builtin_length))
	CHKERROR(addfunc(out, "boolean?", 8, builtin_booleanp))
	CHKERROR(addfunc(out, "number?", 7, builtin_numberp))
	CHKERROR(addfunc(out, "symbol?", 7, builtin_symbolp))
	CHKERROR(addfunc(out, "list?", 5, builtin_listp))
	CHKERROR(addfunc(out, "set-car!", 8, builtin_set_car))
	CHKERROR(addfunc(out, "set-cdr!", 8, builtin_set_cdr))
	CHKERROR(addfunc(out, "pair?", 5, builtin_pairp))
	CHKERROR(addfunc(out, "procedure?", 10, builtin_procedurep))
	CHKERROR(addfunc(out, "string?", 7, builtin_stringp))
	CHKERROR(addfunc(out, "string-append", 13, builtin_string_append))
	CHKERROR(addfunc(out, "string->symbol", 14, builtin_string_symbol))
	CHKERROR(addfunc(out, "symbol->string", 14, builtin_symbol_string))
	CHKERROR(addfunc(out, "string->number", 14, builtin_string_number))
	CHKERROR(addfunc(out, "number->string", 14, builtin_number_string))

	CHKERROR(addfunc(out, "eq?", 3, builtin_eqp))
	CHKERROR(addfunc(out, "neq?", 4, builtin_neqp))
	CHKERROR(addfunc(out, "equal?", 6, builtin_equalp))

	CHKERROR(addsymbol(out, "#f", 2,  &predefined_f))
	CHKERROR(addsymbol(out, "#t", 2,  &predefined_t))
	return ERR_SUCCESS;
}

gserror_t
free_act(hashItem_t * tbl) {
	string_free(&(tbl->name));
	CHKERROR(gc_deref_schemeObject(tbl->value))
	return ERR_SUCCESS;
}

gserror_t
environment_free(environment_t * self)
{
	CHKERROR(hashtable_foreach(&(self->env), (gserror_t (*)(void*))free_act))
	hashtable_free(&(self->env));
	return ERR_SUCCESS;
}

bool
comp(hashItem_t * hi, string_t * str) {
	return string_equals(&(hi->name), str);
}

bool
environment_getObject(environment_t * self, schemeObject_t ** outValue, string_t * str) {
	hashItem_t * hi;
	while (self != NULL) {
		if (hashtable_get(&(self->env), (void **)&hi, str, (int32_t(*)(void *))hashing, (bool (*)(void *, void *))comp))
			goto L_FOUND;
		self = self->parent;
	}
	return false;
L_FOUND:
	*outValue = hi->value;
	if(*outValue != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&((*outValue)->gcInfo)))
	return true;
}

gserror_t
environment_register(environment_t * self, string_t name, schemeObject_t * val)
{
	hashItem_t newhi = {name, val};
	hashItem_t * hi;
	bool ret = hashtable_get(&(self->env), (void **)&hi, &name, (int32_t(*)(void *))hashing, (bool (*)(void *, void *))comp);
	if (ret) {
		if(hi->value != SCHEME_OBJECT_NILL)
			CHKERROR(gc_deref_schemeObject(hi->value))
		memcpy(hi, &newhi, sizeof(hashItem_t));
		return ERR_SUCCESS;
	}
	return hashtable_add(&(self->env), &newhi, sizeof(hashItem_t), (int32_t(*)(void *))hashing);
}

gserror_t
environment_setq3(struct machine * self, environment_t * env, string_t * name, schemeObject_t * val) {
	string_t s;
	string_copy(&s, name);
	if(val != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&(val->gcInfo)))
	CHKERROR(environment_register(env, s, val))
	return ERR_SUCCESS;
}

gserror_t
environment_setq2(struct machine * self, environment_t * env, environment_t * defineto, string_t * name, schemeObject_t * val) {
	schemeObject_t * valres = NULL;
	string_t s;
	CHKERROR(machine_evalforce(self, env, val, &valres)) // valres.rc++;
	string_copy(&s, name);
	CHKERROR(environment_register(defineto, s, valres))
	return ERR_SUCCESS;
}

gserror_t
environment_setq(struct machine * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	schemeObject_t * car = NULL, * cdr = NULL, * cadr = NULL, * caar = NULL, * cdar = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	if (schemeObject_length(val) != 2) {
		errorOut("ERROR", "define", "requires 2 - length list.");
		CHKERROR(gc_deref_schemeObject(val))
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(schemeObject_car(val, &car))
	switch (car->kind) {
	case SCHEME_OBJECT_SYMBOL:
		CHKERROR(schemeObject_cdr(val, &cdr))
		CHKERROR(schemeObject_car(cdr, &cadr))
		CHKERROR(environment_setq2(self, env, env, &car->value.strValue, cadr))
		CHKERROR(gc_deref_schemeObject(cadr))
		CHKERROR(gc_deref_schemeObject(cdr))
		CHKERROR(gc_deref_schemeObject(val))
		out->kind = EVALUATIONRESULT_EVALUATED;
		out->value.evaluatedValue = car;
		return ERR_SUCCESS;
	case SCHEME_OBJECT_CONS:
		CHKERROR(schemeObject_car(car, &caar))
		CHKERROR(gc_deref_schemeObject(car))
		if (caar->kind != SCHEME_OBJECT_SYMBOL) {
			errorOut("ERROR", "define", "(car (1st argument)) must be symbol.");
			CHKERROR(gc_deref_schemeObject(caar))
			CHKERROR(gc_deref_schemeObject(val))
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		CHKERROR(schemeObject_cdr(car, &cdar))
		CHKERROR(schemeObject_cdr(val, &cdr))
		schemeObject_t * lambdaObj = NULL;
		{
			evaluationResult_t er = { 0 };
			lambdaObj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
			if (lambdaObj == NULL) return ERR_OUT_OF_MEMORY;
			CHKERROR(schemeObject_new_cons(lambdaObj, cdar, cdr))
			CHKERROR(gc_ref(&(lambdaObj->gcInfo)))
			CHKERROR(machine_lambda(self, env, lambdaObj, &er))
			CHKERROR(gc_deref_schemeObject(lambdaObj))
			CHKERROR(machine_makeforce(self, er, &lambdaObj))
		}
		CHKERROR(environment_setq2(self, env, env, &caar->value.strValue, lambdaObj));
		CHKERROR(gc_deref_schemeObject(lambdaObj))
		//CHKERROR(gc_deref_schemeObject(cdar))  // <- schemeObject_new_cons doesn't ref count up.
		//CHKERROR(gc_deref_schemeObject(cdr))   // <- schemeObject_new_cons doesn't ref count up.
		CHKERROR(gc_deref_schemeObject(val))
		out->kind = EVALUATIONRESULT_EVALUATED;
		out->value.evaluatedValue = caar;
		return ERR_SUCCESS;
	default:
		errorOut("ERROR", "define", "1st argument must be symbol or list.");
		CHKERROR(gc_deref_schemeObject(car))
		CHKERROR(gc_deref_schemeObject(val))
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	return ERR_SUCCESS;
}

gserror_t
environment_let_internal(struct machine * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out, bool isAster) {
	schemeObject_t * cur = NULL, * cdr = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	if(!schemeObject_isList(val)) {
		errorOut("ERROR", "let", "proper list.");
		CHKERROR(gc_deref_schemeObject(val))
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(schemeObject_car(val, &cur))
	environment_t * the_env = NULL;
	the_env = (environment_t *)reallocarray(NULL, 1, sizeof(environment_t));
	if(the_env == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(environment_new(the_env, env))
	CHKERROR(gc_ref(&(env->gcInfo)))
	CHKERROR(gc_ref(&(the_env->gcInfo)))
	while (cur != SCHEME_OBJECT_NILL) {
		schemeObject_t * prev = cur, * cur_val = NULL, * car = NULL, * cadr = NULL;
		if (cur->kind != SCHEME_OBJECT_CONS) {
			errorOut("ERROR", "let", "invalid format.");
			CHKERROR(gc_deref_schemeObject(cur))
			CHKERROR(gc_deref_schemeObject(val))
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		CHKERROR(schemeObject_car(cur, &cur_val))
		if (schemeObject_length(cur_val) != 2) {
			errorOut("ERROR", "let", "invalid format.");
			CHKERROR(gc_deref_schemeObject(cur_val))
			CHKERROR(gc_deref_schemeObject(cur))
			CHKERROR(gc_deref_schemeObject(val))
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		CHKERROR(schemeObject_car(cur_val, &car))
		CHKERROR(schemeObject_cadr(cur_val, &cadr))
		CHKERROR(environment_setq2(self, isAster ? the_env : env, the_env, &car->value.strValue, cadr))
		CHKERROR(gc_deref_schemeObject(car))
		CHKERROR(gc_deref_schemeObject(cadr))
		CHKERROR(gc_deref_schemeObject(cur_val))

		CHKERROR(schemeObject_cdr(cur, &cur))
		CHKERROR(gc_deref_schemeObject(prev))
	}
	CHKERROR(schemeObject_cdr(val, &cdr))
	CHKERROR(machine_begin(self, the_env, cdr, out))
	CHKERROR(gc_deref_environment(the_env))
	CHKERROR(gc_deref_schemeObject(cdr))
	CHKERROR(gc_deref_schemeObject(val))
	return ERR_SUCCESS;
}

gserror_t
environment_let(struct machine * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	return environment_let_internal(self, env, val, out, false);
}

gserror_t
environment_letaster(struct machine * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	return environment_let_internal(self, env, val, out, true);
}

gserror_t
environment_set_destructive(struct machine * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out)
{
	schemeObject_t * car = NULL, * cdr = NULL, * cadr = NULL, * cadrres = NULL;
	gserror_t ret = ERR_SUCCESS;
	CHKERROR(gc_ref(&(val->gcInfo)))
	if (schemeObject_length(val) != 2) {
		errorOut("ERROR", "define", "requires 2 - length list.");
		CHKERROR(gc_deref_schemeObject(val))
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(schemeObject_car(val, &car))
	if (car->kind != SCHEME_OBJECT_SYMBOL) {
		errorOut("ERROE", "define", "1st argument must be symbol.");
		CHKERROR(gc_deref_schemeObject(car))
		CHKERROR(gc_deref_schemeObject(val))
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	CHKERROR(schemeObject_cdr(val, &cdr))
	CHKERROR(schemeObject_car(cdr, &cadr))
	CHKERROR(machine_evalforce(self, env, cadr, &cadrres)) // cadrres.rc++;
	hashItem_t * hi;
	environment_t * current = env;
	while(current != NULL) {
		bool ret = hashtable_get(&(current->env), (void **)&hi, &car->value.strValue, (int32_t(*)(void *))hashing, (bool (*)(void *, void *))comp);
		if (ret) {
			if(hi->value != SCHEME_OBJECT_NILL)
				CHKERROR(gc_deref_schemeObject(hi->value))
			memcpy(&(hi->value), &cadrres, sizeof(schemeObject_t *));
			break;
		}
		current = current->parent;
	}
	if(current == NULL) {
		fprintf(stderr, "[ERROR] set!: Not found name:");
		string_writeLine(stderr, &car->value.strValue);
		ret = ERR_EVAL_NOT_FOUND_SYMBOL;
		CHKERROR(gc_deref_schemeObject(cadrres));
	}
	CHKERROR(gc_deref_schemeObject(cadr))
	CHKERROR(gc_deref_schemeObject(cdr))
	CHKERROR(gc_deref_schemeObject(val))
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = car;
	return ret;
}
