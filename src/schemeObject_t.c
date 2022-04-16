#include "schemeObject_t.h"
#include "list.h"
#include "common.h"
#include <stdio.h>

error_t
schemeObject_new_string(schemeObject_t * out, string_t str) {
	out->kind = SCHEME_OBJECT_STRING;
	out->value.strValue = str;
	return ERR_SUCCESS;
}

error_t
schemeObject_new_number(schemeObject_t * out, int32_t num) {
	out->kind = SCHEME_OBJECT_NUMBER;
	out->value.numValue = num;
	return ERR_SUCCESS;
}

error_t
schemeObject_new_symbol(schemeObject_t * out, string_t sym) {
	out->kind = SCHEME_OBJECT_SYMBOL;
	out->value.symValue = sym;
	return ERR_SUCCESS;
}

error_t
schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next) {
	out->kind = SCHEME_OBJECT_CONS;
	out->value.consValue.value = value;
	out->value.consValue.next = next;
	return ERR_SUCCESS;
}

error_t
schemeObject_new_extFunc(schemeObject_t * out, struct environment * environment, error_t (*func)(struct machine *, struct environment *, schemeObject_t *, schemeObject_t **)) {
	out->kind = SCHEME_OBJECT_EXTERN_FUNCTION;
	out->value.extFuncValue.environment = environment;
	out->value.extFuncValue.func = func;
	return ERR_SUCCESS;
}

error_t
schemeObject_car(schemeObject_t * self, schemeObject_t ** out)
{
	if (self->kind != SCHEME_OBJECT_CONS) return ERR_EVAL_INVALID_OBJECT_TYPE;
	*out = self->value.consValue.value;
	return ERR_SUCCESS;
}

error_t
schemeObject_cdr(schemeObject_t * self, schemeObject_t ** out)
{
	if (self->kind != SCHEME_OBJECT_CONS) return ERR_EVAL_INVALID_OBJECT_TYPE;
	*out = self->value.consValue.next;
	return ERR_SUCCESS;
}

error_t
schemeObject_quote(struct machine * self, struct environment * env, schemeObject_t * val, schemeObject_t ** out) {
	if (val->kind != SCHEME_OBJECT_CONS) return ERR_EVAL_INVALID_OBJECT_TYPE;
	*out = val->value.consValue.value;
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
					CHKERROR(stringBuilder_append(&sb, "<FUNCTION>", 10))
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
