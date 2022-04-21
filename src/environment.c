#include "environment.h"
#include "machine.h"
#include "string_t.h"
#include "gc.h"
#include <stdio.h>

typedef struct hashItem {
	string_t name;
	schemeObject_t * value;
} hashItem_t;

int32_t
hashing(hashItem_t * hi) {
	return string_hash(&(hi->name));
}

error_t
environment_new(environment_t * out, environment_t * parent) {
	out->parent = parent;
	gcInfo_new(&out->gcInfo);
	return hashtable_new(&(out->env));
}

error_t
copyaction(hashItem_t * inplace) {
	schemeObject_t * sobj = inplace->value;
	string_t s = inplace->name;
	CHKERROR(schemeObject_copy(&(inplace->value), sobj))
	CHKERROR(string_copy(&(inplace->name), &s))
	return ERR_SUCCESS;
}

error_t
environment_clone(environment_t * out, environment_t * inenv) {
	environment_new(out, inenv->parent);
	if(inenv->parent != NULL)
		CHKERROR(gc_ref(&(inenv->parent->gcInfo)))
	CHKERROR(hashtable_copy(&(out->env), &(inenv->env), sizeof(schemeObject_t)))
	CHKERROR(hashtable_foreach(&(out->env), (error_t (*)(void *)) copyaction));
	return ERR_SUCCESS;
}

error_t
addfunc(environment_t * out, char * name, size_t name_length, error_t(*func)(struct machine *, struct environment *, schemeObject_t *, schemeObject_t **)) {
	string_t str;
	string_new_deep(&str, name, name_length);
	schemeObject_t * obj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (obj == NULL) return ERR_OUT_OF_MEMORY;
	schemeObject_new_extFunc(obj, out, func);
	CHKERROR(gc_ref(&(out->gcInfo)))
	CHKERROR(gc_ref(&(obj->gcInfo)))
	environment_register(out, str, obj);
	return ERR_SUCCESS;
}

error_t
environment_new_global(environment_t * out)
{
	CHKERROR(environment_new(out, NULL))
	CHKERROR(gc_ref(&(out->gcInfo)))
	// TODO:Implement buildin function.
	CHKERROR(addfunc(out, "define", 6, environment_setq));
	CHKERROR(addfunc(out, "quote", 5, schemeObject_quote));
	CHKERROR(addfunc(out, "car", 3, schemeObject_car2));
	CHKERROR(addfunc(out, "cdr", 3, schemeObject_cdr2));
	CHKERROR(addfunc(out, "cons", 4, schemeObject_cons));
	CHKERROR(addfunc(out, "lambda", 6, machine_lambda));
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
	CHKERROR(gc_ref(&((*outValue)->gcInfo)))
	return true;
}

error_t
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

error_t
environment_setq2(struct machine * self, environment_t * env, string_t * name, schemeObject_t * val) {
	schemeObject_t * valres = NULL;
	string_t s;
	CHKERROR(machine_eval(self, env, val, &valres)) // valres.rc++;
	string_copy(&s, name);
	CHKERROR(environment_register(env, s, valres))
	return ERR_SUCCESS;
}

error_t
environment_setq(struct machine * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out)
{
	schemeObject_t * car = NULL, * cdr = NULL, * cadr = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	if (!schemeObject_isListLimited(val, 2)) {
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
	CHKERROR(environment_setq2(self, env, &car->value.strValue, cadr))
	CHKERROR(gc_deref_schemeObject(cadr))
	CHKERROR(gc_deref_schemeObject(cdr))
	CHKERROR(gc_deref_schemeObject(val))
	* out = car;
	return ERR_SUCCESS;
}
