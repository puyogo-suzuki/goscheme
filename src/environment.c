#include "environment.h"
#include "machine.h"
#include "string_t.h"

typedef struct hashItem {
	string_t name;
	schemeObject_t * value;
} hashItem_t;

int32_t
hashing(hashItem_t * hi) {
	return string_hash(&(hi->name));
}

error_t
environment_new(environment_t * out, environment_t * parent)
{
	out->parent = parent;
	return hashtable_new(&(out->env));
}

error_t
addfunc(environment_t * out, char * name, size_t name_length, error_t(*func)(struct machine *, struct environment *, schemeObject_t *, schemeObject_t **)) {
	string_t str;
	string_new_deep(&str, name, name_length);
	schemeObject_t * obj = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (obj == NULL) return ERR_OUT_OF_MEMORY;
	schemeObject_new_extFunc(obj, out, func);
	environment_register(out, str, obj);
}

error_t
environment_new_global(environment_t * out)
{
	CHKERROR(environment_new(out, NULL))
	// TODO:Implement buildin function.
	CHKERROR(addfunc(out, "define", 6, environment_setq));
	CHKERROR(addfunc(out, "quote", 5, schemeObject_quote));
	return ERR_SUCCESS;
}

bool
comp(hashItem_t * hi, string_t * str)
{
	return string_equals(&(hi->name), str);
}

bool
environment_getObject(environment_t * self, schemeObject_t ** outValue, string_t * str)
{
	hashItem_t * hi;
	bool ret = hashtable_get(&(self->env), (void **)&hi, str, (int32_t(*)(void *))hashing, (bool (*)(void *, void *))comp);
	if (!ret) return false;
	*outValue = hi->value;
	return ret;
}

error_t
environment_register(environment_t * self, string_t name, schemeObject_t * val)
{
	hashItem_t newhi = {name, val};
	hashItem_t * hi;
	bool ret = hashtable_get(&(self->env), (void **)&hi, &name, (int32_t(*)(void *))hashing, (bool (*)(void *, void *))comp);
	if (ret) {
		memcpy(hi, &newhi, sizeof(hashItem_t));
		return ERR_SUCCESS;
	}
	return hashtable_add(&(self->env), &newhi, sizeof(hashItem_t), (int32_t(*)(void *))hashing);
}

error_t
environment_setq(struct machine * self, environment_t * env, schemeObject_t * val, schemeObject_t ** out)
{
	schemeObject_t * car, * cdr, * cdrres;
	string_t s;
	CHKERROR(schemeObject_car(val, &car))
	if (car->kind != SCHEME_OBJECT_SYMBOL)
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	CHKERROR(schemeObject_cdr(val, &cdr))
	CHKERROR(machine_eval(self, env, &cdrres, cdr->value.consValue.value))
	CHKERROR(string_copy(&s, &car->value.strValue))
	CHKERROR(environment_register(env, s, cdrres))
	* out = cdrres;
	return ERR_SUCCESS;
}
