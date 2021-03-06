#include "schemeObject_t.h"
#include "list.h"
#include "common.h"
#include "machine.h"
#include <stdio.h>

gserror_t
schemeObject_new_string(schemeObject_t * out, string_t str) {
	out->kind = SCHEME_OBJECT_STRING;
	out->value.strValue = str;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
schemeObject_new_number(schemeObject_t * out, int32_t num) {
	out->kind = SCHEME_OBJECT_NUMBER;
	out->value.numValue = num;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
schemeObject_new_symbol(schemeObject_t * out, string_t sym) {
	out->kind = SCHEME_OBJECT_SYMBOL;
	out->value.symValue = sym;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next) {
	out->kind = SCHEME_OBJECT_CONS;
	out->value.consValue.value = value;
	out->value.consValue.next = next;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
schemeObject_new_extFunc(schemeObject_t * out, struct environment * environment, schemeFunction_t * func) {
	out->kind = SCHEME_OBJECT_EXTERN_FUNCTION;
	out->value.extFuncValue.environment = environment;
	out->value.extFuncValue.func = func;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
schemeObject_new_procedure(schemeObject_t * out, struct environment * environment, schemeObject_t * body) {
	out->kind = SCHEME_OBJECT_PROCEDURE;
	out->value.procedureValue.environment = environment;
	out->value.procedureValue.body = body;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
schemeObject_new_macro(schemeObject_t * out, struct environment * environment, schemeObject_t * body) {
	out->kind = SCHEME_OBJECT_MACRO;
	out->value.macroValue.environment = environment;
	out->value.macroValue.body = body;
	gcInfo_new(&out->gcInfo);
	return ERR_SUCCESS;
}

gserror_t
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
		case SCHEME_OBJECT_MACRO:
			if (inobj->value.procedureValue.body != SCHEME_OBJECT_NILL)
				gc_ref(&(inobj->value.procedureValue.body->gcInfo));
			break;
	}
	return ERR_SUCCESS;
}

typedef struct clone_env {
	schemeObject_t ** writeTo;
	schemeObject_t * readFrom;
} clone_env_t;

gserror_t
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

int32_t
schemeObject_length(schemeObject_t * self) {
	schemeObject_t * cursor = self;
	int32_t i = 0;
	for(i = 0; cursor != SCHEME_OBJECT_NILL; ++i) {
		if(cursor->kind != SCHEME_OBJECT_CONS) return -1;
		cursor = cursor->value.consValue.next;
	}
	return i;
}

gserror_t
schemeObject_car(schemeObject_t * self, schemeObject_t ** out) {
	if(self == SCHEME_OBJECT_NILL || self->kind != SCHEME_OBJECT_CONS) {
		errorOut("ERROR", "car", "car requires CONS cell.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	*out = self->value.consValue.value;
	if(*out != NULL) CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_cdr(schemeObject_t * self, schemeObject_t ** out) {
	if (self == SCHEME_OBJECT_NILL || self->kind != SCHEME_OBJECT_CONS) {
		errorOut("ERROR", "cdr", "cdr requires CONS cell.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	*out = self->value.consValue.next;
	if(*out != NULL) CHKERROR(gc_ref(&((*out)->gcInfo)))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_cadr(schemeObject_t * self, schemeObject_t ** out) {
	schemeObject_t * cdr;
	CHKERROR(schemeObject_cdr(self, &cdr))
	CHKERROR(schemeObject_car(cdr, out))
	CHKERROR(gc_deref_schemeObject(cdr))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_cddr(schemeObject_t * self, schemeObject_t ** out) {
	schemeObject_t * cdr;
	CHKERROR(schemeObject_cdr(self, &cdr))
	CHKERROR(schemeObject_cdr(cdr, out))
	CHKERROR(gc_deref_schemeObject(cdr))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_caddr(schemeObject_t * self, schemeObject_t ** out) {
	schemeObject_t * cddr;
	CHKERROR(schemeObject_cddr(self, &cddr))
	CHKERROR(schemeObject_car(cddr, out))
	CHKERROR(gc_deref_schemeObject(cddr))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_cdddr(schemeObject_t * self, schemeObject_t ** out) {
	schemeObject_t * cddr;
	CHKERROR(schemeObject_cddr(self, &cddr))
	CHKERROR(schemeObject_cdr(cddr, out))
	CHKERROR(gc_deref_schemeObject(cddr))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_cadddr(schemeObject_t * self, schemeObject_t ** out) {
	schemeObject_t * cdddr;
	CHKERROR(schemeObject_cdddr(self, &cdddr))
	CHKERROR(schemeObject_car(cdddr, out))
	CHKERROR(gc_deref_schemeObject(cdddr))
	return ERR_SUCCESS;
}

gserror_t
schemeObject_cddddr(schemeObject_t * self, schemeObject_t ** out) {
	schemeObject_t * cdddr;
	CHKERROR(schemeObject_cdddr(self, &cdddr))
	CHKERROR(schemeObject_cdr(cdddr, out))
	CHKERROR(gc_deref_schemeObject(cdddr))
	return ERR_SUCCESS;
}

bool
schemeObject_eqp(schemeObject_t * lhs, schemeObject_t * rhs) {
	if(lhs == rhs) return true;
	if(lhs == SCHEME_OBJECT_NILL || rhs == SCHEME_OBJECT_NILL) return false;
	if(lhs->kind != rhs->kind) return false;
	switch (lhs->kind) {
	case SCHEME_OBJECT_EXTERN_FUNCTION:
		return lhs->value.extFuncValue.func == rhs->value.extFuncValue.func
			&& lhs->value.extFuncValue.environment == rhs->value.extFuncValue.environment;
	case SCHEME_OBJECT_NUMBER:
		return lhs->value.numValue == rhs->value.numValue;
	case SCHEME_OBJECT_PROCEDURE:
		return lhs->value.procedureValue.body == rhs->value.procedureValue.body
			&& lhs->value.procedureValue.environment == rhs->value.procedureValue.environment;
	case SCHEME_OBJECT_CONS:
		return lhs->value.consValue.value == rhs->value.consValue.value
			&& lhs->value.consValue.next == rhs->value.consValue.next;
	case SCHEME_OBJECT_STRING:
		return lhs->value.strValue.buffer == rhs->value.strValue.buffer;
	case SCHEME_OBJECT_SYMBOL:
		return lhs->value.symValue.buffer == rhs->value.symValue.buffer;
	case SCHEME_OBJECT_MACRO:
		return lhs->value.macroValue.body == rhs->value.macroValue.body;
	}
	return false;
}

bool
schemeObject_equalp(schemeObject_t * lhs, schemeObject_t * rhs) {
	if(lhs == rhs) return true;
	if(lhs == SCHEME_OBJECT_NILL || rhs == SCHEME_OBJECT_NILL) return false;
	if(lhs->kind != rhs->kind) return false;
	switch (lhs->kind) {
	case SCHEME_OBJECT_EXTERN_FUNCTION: case SCHEME_OBJECT_NUMBER: case SCHEME_OBJECT_PROCEDURE: case SCHEME_OBJECT_MACRO:
		return schemeObject_eqp(lhs, rhs);
	case SCHEME_OBJECT_CONS:
		return schemeObject_equalp(lhs->value.consValue.value, rhs->value.consValue.value)
			&& schemeObject_equalp(lhs->value.consValue.next, rhs->value.consValue.next);
	case SCHEME_OBJECT_STRING:
		return string_equals(&(lhs->value.strValue), &(rhs->value.strValue));
	case SCHEME_OBJECT_SYMBOL:
		return string_equals(&(lhs->value.symValue), &(rhs->value.symValue));
	}
	return false;
}

gserror_t
schemeObject_map(struct machine * self, struct environment * env, schemeObject_t ** out, schemeObject_t * inobj, schemeFunction_t * mapper) {
	schemeObject_t ** writeTo = out;
	schemeObject_t * readFrom = inobj;
	*out = SCHEME_OBJECT_NILL;
	while(readFrom != SCHEME_OBJECT_NILL) {
		evaluationResult_t evres;
		if(readFrom->kind != SCHEME_OBJECT_CONS) {
			errorOut("ERROR", "map", "must be LIST");
			return ERR_EVAL_INVALID_OBJECT_TYPE;
		}
		*writeTo = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
		if(*writeTo == NULL) return ERR_OUT_OF_MEMORY;
		CHKERROR(schemeObject_new_cons2(*writeTo, SCHEME_OBJECT_NILL))
		CHKERROR(gc_ref(&(*writeTo)->gcInfo)) // TODO: check gc ref of tail!
		CHKERROR(mapper(self, env, readFrom->value.consValue.value, &evres))
		CHKERROR(machine_makeforce(self, evres, &(*writeTo)->value.consValue.value))
		writeTo = &((*writeTo)->value.consValue.next);
		readFrom = readFrom->value.consValue.next;
	}
	return ERR_SUCCESS;
}


gserror_t
schemeObject_quote(struct machine * self, struct environment * env, schemeObject_t * val, evaluationResult_t * out) {
	if (schemeObject_length(val) != 1) {
		errorOut("ERROR", "quote", "quote requires 1 argument.");
		return ERR_EVAL_INVALID_OBJECT_TYPE;
	}
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = val->value.consValue.value;
	if(out->value.evaluatedValue != SCHEME_OBJECT_NILL)
		CHKERROR(gc_ref(&(out->value.evaluatedValue->gcInfo)))
	
	return ERR_SUCCESS;
}

typedef struct toStringEnv {
	schemeObject_t * cur;
	bool dot;
} toStringEnv_t;

gserror_t
schemeObject_toString(string_t * out, schemeObject_t * inobj) {
	stringBuilder_t sb;
	linkedList_t * stack = NULL;
	toStringEnv_t env = {inobj, false};
	linkedList_add2(&stack, &env, toStringEnv_t);
	CHKERROR(stringBuilder_new(&sb))
	if(env.cur == SCHEME_OBJECT_NILL) {
		CHKERROR(stringBuilder_append(&sb, "'()", 3))
		goto L_END;
	}
	if(env.cur->kind == SCHEME_OBJECT_CONS) {
		CHKERROR(stringBuilder_append(&sb, "(", 1))
		env.cur = env.cur->value.consValue.value;
		if(env.cur->kind == SCHEME_OBJECT_CONS)
			CHKERROR(stringBuilder_append(&sb, "(", 1))
	}
	linkedList_pop2(&stack, &env, toStringEnv_t);
	{ // do-while?????????????????????????????????????????????????????????????????????????????????????????????
		L_START:
		if(env.cur == SCHEME_OBJECT_NILL)
			CHKERROR(stringBuilder_append(&sb, ")", 1))
		if(env.dot)
			CHKERROR(stringBuilder_append(&sb, ". ", 2))
		while(env.cur != SCHEME_OBJECT_NILL) {
			switch(env.cur->kind) {
				case SCHEME_OBJECT_SYMBOL:
					CHKERROR(stringBuilder_append2(&sb, &(env.cur->value.strValue)))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_STRING:
					CHKERROR(stringBuilder_append(&sb, "\"", 1))
					CHKERROR(stringBuilder_append2(&sb, &(env.cur->value.strValue)))
					CHKERROR(stringBuilder_append(&sb, "\"", 1))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_NUMBER: {
					char buf[32];
					sprintf(buf, "%d", env.cur->value.numValue);
					CHKERROR(stringBuilder_append(&sb, buf, strlen(buf)));
					goto CONTINUE_OUTER_WHILE;
				}
				case SCHEME_OBJECT_CONS: {
					toStringEnv_t ne = {env.cur->value.consValue.next, false};
					if(ne.cur != SCHEME_OBJECT_NILL && ne.cur->kind != SCHEME_OBJECT_CONS)
						ne.dot = true;
					linkedList_add2(&stack, &ne, toStringEnv_t);
					env.cur = env.cur->value.consValue.value;
					if(env.cur == SCHEME_OBJECT_NILL) {
						CHKERROR(stringBuilder_append(&sb, "NIL", 3))
					}else if (env.cur->kind == SCHEME_OBJECT_CONS) CHKERROR(stringBuilder_append(&sb, "(", 1))
					/*else if (cur->kind == SCHEME_OBJECT_CONS) {
						if (cur->value.consValue.value->kind == SCHEME_OBJECT_SYMBOL && string_equals2(&cur->value.consValue.value->value.symValue, "quote", 5)) {
							CHKERROR(stringBuilder_append(&sb, "'", 1))
							cur = cur->value.consValue.next;
						}
						else
							CHKERROR(stringBuilder_append(&sb, "(", 1))
					}*/ // ????????????
					break;
				}
				case SCHEME_OBJECT_EXTERN_FUNCTION:
					CHKERROR(stringBuilder_append(&sb, "<EXTFUNCT>", 10))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_PROCEDURE:
					CHKERROR(stringBuilder_append(&sb, "<PROCEDURE>", 11))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_MACRO:
					CHKERROR(stringBuilder_append(&sb, "<MACRO>", 7))
					goto CONTINUE_OUTER_WHILE;
			}
		}
		goto L_SKIP;
	CONTINUE_OUTER_WHILE:
		if(env.dot) {
			CHKERROR(stringBuilder_append(&sb, ")", 1))
			env.dot = false;
		}
	L_SKIP:
		if(!linkedList_pop2(&stack, &env.cur, toStringEnv_t)) goto L_END;
		if(env.cur != SCHEME_OBJECT_NILL)
			CHKERROR(stringBuilder_append(&sb, " ", 1))
		goto L_START;
	}
	L_END:
	CHKERROR(stringBuilder_toString(out, &sb))
	stringBuilder_free(&sb);
	return ERR_SUCCESS;
}
