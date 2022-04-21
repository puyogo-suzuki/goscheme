#include "schemeObject_t.h"
#include "list.h"
#include "common.h"
#include "machine.h"
#include <stdio.h>

error_t
schemeObject_new_string(schemeObject_t * out, string_t str) {
	out->kind = SCHEME_OBJECT_STRING;
	out->value.strValue = str;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

error_t
schemeObject_new_number(schemeObject_t * out, int32_t num) {
	out->kind = SCHEME_OBJECT_NUMBER;
	out->value.numValue = num;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

error_t
schemeObject_new_symbol(schemeObject_t * out, string_t sym) {
	out->kind = SCHEME_OBJECT_SYMBOL;
	out->value.symValue = sym;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

error_t
schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next) {
	out->kind = SCHEME_OBJECT_CONS;
	out->value.consValue.value = value;
	out->value.consValue.next = next;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

error_t
schemeObject_new_extFunc(schemeObject_t * out, struct environment * environment, error_t (*func)(struct machine *, struct environment *, schemeObject_t *, schemeObject_t **)) {
	out->kind = SCHEME_OBJECT_EXTERN_FUNCTION;
	out->value.extFuncValue.environment = environment;
	out->value.extFuncValue.func = func;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

error_t
schemeObject_new_procedure(schemeObject_t * out, struct environment * environment, schemeObject_t * body) {
	out->kind = SCHEME_OBJECT_PROCEDURE;
	out->value.procedureValue.environment = environment;
	out->value.procedureValue.body = body;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

error_t
schemeObject_copy_onedepth(schemeObject_t ** out, schemeObject_t * inobj) {
	if(inobj == SCHEME_OBJECT_NILL) {
		*out = SCHEME_OBJECT_NILL;
		return ERR_SUCCESS;
	}
	*out = (schemeObject_t *) reallocarray(NULL, 1, sizeof(schemeObject_t));
	if(*out == NULL) return ERR_OUT_OF_MEMORY;
	gcInfo_new(&((*out)->gcInfo));
	(*out)->kind = inobj->kind;
	(*out)->value = inobj->value;
	switch(inobj->kind) {
		case SCHEME_OBJECT_CONS:
		case SCHEME_OBJECT_NUMBER:
			break;
		case SCHEME_OBJECT_EXTERN_FUNCTION:
			if(inobj->value.extFuncValue.environment != NULL)
				gc_ref(&(inobj->value.extFuncValue.environment->gcInfo));
			break;
		case SCHEME_OBJECT_PROCEDURE:
			if(inobj->value.procedureValue.environment != NULL)
				gc_ref(&(inobj->value.procedureValue.environment->gcInfo));
			if(inobj->value.procedureValue.body != SCHEME_OBJECT_NILL)
				gc_ref(&(inobj->value.procedureValue.body->gcInfo));
			break;
		case SCHEME_OBJECT_SYMBOL:
			string_copy(&((*out)->value.symValue), &inobj->value.symValue);
			break;
		case SCHEME_OBJECT_STRING:
			string_copy(&((*out)->value.strValue), &inobj->value.strValue);
			break;
	}
	return ERR_SUCCESS;
}

typedef struct clone_env {
	schemeObject_t ** writeTo;
	schemeObject_t * readFrom;
} clone_env_t;

error_t
schemeObject_copy(schemeObject_t ** out, schemeObject_t * inobj) { // *out's reference count is 1.
	if(inobj == SCHEME_OBJECT_NILL) {
		*out = SCHEME_OBJECT_NILL;
		return ERR_SUCCESS;
	}
	if(inobj->kind != SCHEME_OBJECT_CONS) // fast path.
		return schemeObject_copy_onedepth(out, inobj);
	clone_env_t env = {out, inobj};
	linkedList_t * envStack = NULL;
	CHKERROR(linkedList_add2(&envStack, &env, clone_env_t))
	while(linkedList_pop2(&envStack, &env, clone_env_t)) {
		while(env.readFrom != NULL) {
			schemeObject_copy_onedepth(env.writeTo, env.readFrom);
			CHKERROR(gc_ref(&((*env.writeTo)->gcInfo)))
			if(env.readFrom->kind == SCHEME_OBJECT_CONS) {
				clone_env_t newenv = {&((*env.writeTo)->value.consValue.next), env.readFrom->value.consValue.next};
				CHKERROR(linkedList_add2(&envStack, &newenv, clone_env_t))
				env.writeTo = &((*env.writeTo)->value.consValue.value);
				env.readFrom = env.readFrom->value.consValue.value;
				continue;
			}
			break;
		}
	}
	return ERR_SUCCESS;
}

bool
schemeObject_isList(schemeObject_t * self) {
	schemeObject_t * cursor = self;
	while(cursor != SCHEME_OBJECT_NILL) {
		if(cursor->kind != SCHEME_OBJECT_CONS) return false;
		cursor = cursor->value.consValue.next;
	}
	return true;
}

bool
schemeObject_isListLimited(schemeObject_t * self, int32_t listLength) {
	schemeObject_t * cursor = self;
	int i = 0;
	for(i = 0; cursor != SCHEME_OBJECT_NILL; ++i) {
		if(cursor->kind != SCHEME_OBJECT_CONS) return false;
		cursor = cursor->value.consValue.next;
	}
	return listLength == i;
}

error_t
schemeObject_cons(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out) {
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
	CHKERROR(machine_eval(self, env, carres, &carevalres))
	CHKERROR(gc_deref_schemeObject(carres))
	CHKERROR(machine_eval(self, env, cadrres, &cadrevalres))
	CHKERROR(gc_deref_schemeObject(cadrres))
	*out = reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (*out == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(schemeObject_new_cons(*out, carevalres, cadrevalres))
	// carevalres, cadrevalresはconsでref incrementしないので，ref decrementしない．
	// *out が参照しているので，それぞれ1ずつあり，うまくいくはずである．
	CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}

error_t
schemeObject_car(schemeObject_t * self, schemeObject_t ** out) {
	if (self->kind != SCHEME_OBJECT_CONS) {
		errorOut("ERROR", "car", "car requires CONS cell.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	*out = self->value.consValue.value;
	if(*out != NULL) CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}

error_t
schemeObject_car2(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out) {
	if (!schemeObject_isListLimited(val, 1)) {
		errorOut("ERROR", "car", "car requires 1 argument which is CONS cell.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	schemeObject_t * carres = NULL, * evalres = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	CHKERROR(schemeObject_car(val, &carres))
	CHKERROR(gc_deref_schemeObject(val))
	CHKERROR(machine_eval(self, env, carres, &evalres))
	CHKERROR(gc_deref_schemeObject(carres))
	CHKERROR(schemeObject_car(evalres, out))
	CHKERROR(gc_deref_schemeObject(evalres))
	return ERR_SUCCESS;
}


error_t
schemeObject_cdr(schemeObject_t * self, schemeObject_t ** out) {
	if (self->kind != SCHEME_OBJECT_CONS) {
		errorOut("ERROR", "cdr", "cdr requires CONS cell.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	*out = self->value.consValue.next;
	if(*out != NULL) CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}

error_t
schemeObject_cdr2(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out) {
	if (!schemeObject_isListLimited(val, 1)) {
		errorOut("ERROR", "cdr", "cdr requires 1 argument which is CONS cell.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	schemeObject_t * carres = NULL, * evalres = NULL;
	CHKERROR(gc_ref(&(val->gcInfo)))
	CHKERROR(schemeObject_car(val, &carres))
	CHKERROR(gc_deref_schemeObject(val))
	CHKERROR(machine_eval(self, env, carres, &evalres))
	CHKERROR(gc_deref_schemeObject(carres))
	CHKERROR(schemeObject_cdr(evalres, out))
	CHKERROR(gc_deref_schemeObject(evalres))
	return ERR_SUCCESS;
}

error_t
schemeObject_map(struct machine * self, struct environment * env, schemeObject_t ** out, schemeObject_t * inobj, error_t (mapper)(struct machine *, struct environment *, schemeObject_t *, schemeObject_t **)) {
	schemeObject_t ** writeTo = out;
	schemeObject_t * readFrom = inobj;
	*out = SCHEME_OBJECT_NILL;
	while(readFrom != SCHEME_OBJECT_NILL) {
		if(readFrom->kind != SCHEME_OBJECT_CONS) {
			errorOut("ERROR", "map", "must be LIST");
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		*writeTo = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
		if(*writeTo == NULL) return ERR_OUT_OF_MEMORY;
		CHKERROR(schemeObject_new_cons2(*writeTo, SCHEME_OBJECT_NILL))
		CHKERROR(gc_ref(&(*writeTo)->gcInfo))
		CHKERROR(mapper(self, env, readFrom->value.consValue.value, &(*writeTo)->value.consValue.value))
		writeTo = &((*writeTo)->value.consValue.next);
		readFrom = readFrom->value.consValue.next;
	}
	return ERR_SUCCESS;
}


error_t
schemeObject_quote(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out) {
	if (!schemeObject_isListLimited(val, 1)) {
		errorOut("ERROR", "quote", "quote requires 1 argument.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	*out = val->value.consValue.value;
	CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}

error_t
schemeObject_toString(string_t * out, schemeObject_t * inobj) {
	stringBuilder_t sb;
	linkedList_t * stack = NULL;
	linkedList_add2(&stack, &inobj, schemeObject_t *);
	CHKERROR(stringBuilder_new(&sb))
	schemeObject_t * cur = inobj;
	if(cur->kind == SCHEME_OBJECT_CONS) {
		CHKERROR(stringBuilder_append(&sb, "(", 1))
		cur = cur->value.consValue.value;
		if(cur->kind == SCHEME_OBJECT_CONS)
			CHKERROR(stringBuilder_append(&sb, "(", 1))
	}
	linkedList_pop2(&stack, &cur, schemeObject_t *);
	{ // do-whileにしたかったけど、ダメでした（非効率なコード書けば変形できる）
		L_START:
		if(cur == SCHEME_OBJECT_NILL)
			CHKERROR(stringBuilder_append(&sb, ")", 1))
		while(cur != SCHEME_OBJECT_NILL) {
			switch(cur->kind) {
				case SCHEME_OBJECT_SYMBOL:
					CHKERROR(stringBuilder_append2(&sb, &(cur->value.strValue)))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_STRING:
					CHKERROR(stringBuilder_append(&sb, "\"", 1))
					CHKERROR(stringBuilder_append2(&sb, &(cur->value.strValue)))
					CHKERROR(stringBuilder_append(&sb, "\"", 1))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_NUMBER: {
					char buf[32];
					sprintf(buf, "%d", cur->value.numValue);
					CHKERROR(stringBuilder_append(&sb, buf, strlen(buf)));
					goto CONTINUE_OUTER_WHILE;
				}
				case SCHEME_OBJECT_CONS: {
					schemeObject_t * ne = cur->value.consValue.next;
					if(ne != SCHEME_OBJECT_NILL && ne->kind != SCHEME_OBJECT_CONS)
						CHKERROR(stringBuilder_append(&sb, ".", 1))
					linkedList_add2(&stack, &ne, schemeObject_t *);
					cur = cur->value.consValue.value;
					if(cur == SCHEME_OBJECT_NILL) {
						CHKERROR(stringBuilder_append(&sb, "NIL", 3))
					}else if (cur->kind == SCHEME_OBJECT_CONS) CHKERROR(stringBuilder_append(&sb, "(", 1))
					/*else if (cur->kind == SCHEME_OBJECT_CONS) {
						if (cur->value.consValue.value->kind == SCHEME_OBJECT_SYMBOL && string_equals2(&cur->value.consValue.value->value.symValue, "quote", 5)) {
							CHKERROR(stringBuilder_append(&sb, "'", 1))
							cur = cur->value.consValue.next;
						}
						else
							CHKERROR(stringBuilder_append(&sb, "(", 1))
					}*/ // 実装面倒
					break;
				}
				case SCHEME_OBJECT_EXTERN_FUNCTION:
					CHKERROR(stringBuilder_append(&sb, "<EXTFUNCT>", 10))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_PROCEDURE:
					CHKERROR(stringBuilder_append(&sb, "<PROCEDURE>", 11))
					goto CONTINUE_OUTER_WHILE;
			}
		}
		CONTINUE_OUTER_WHILE:
		if(!linkedList_pop2(&stack, &cur, schemeObject_t *)) goto L_END;
		if(cur != SCHEME_OBJECT_NILL)
			CHKERROR(stringBuilder_append(&sb, " ", 1))
		goto L_START;
	}
	L_END:
	CHKERROR(stringBuilder_toString(out, &sb))
	stringBuilder_free(&sb);
	return ERR_SUCCESS;
}
