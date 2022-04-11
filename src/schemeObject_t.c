#include "schemeObject_t.h"
#include "list.h"
#include "common.h"
#include <stdio.h>

#define CHKFAIL(s) if(!(s)) return false;

bool
schemeObject_new_string(schemeObject_t * out, string_t str) {
	out->kind = SCHEME_OBJECT_STRING;
	out->value.strValue = str;
	return true;
}

bool
schemeObject_new_number(schemeObject_t * out, int32_t num) {
	out->kind = SCHEME_OBJECT_NUMBER;
	out->value.numValue = num;
	return true;
}

bool
schemeObject_new_symbol(schemeObject_t * out, string_t sym) {
	out->kind = SCHEME_OBJECT_SYMBOL;
	out->value.symValue = sym;
	return true;
}

bool
schemeObject_new_cons(schemeObject_t * out, schemeObject_t * value, schemeObject_t * next) {
	out->kind = SCHEME_OBJECT_CONS;
	out->value.consValue.value = value;
	out->value.consValue.next = next;
	return true;
}

bool
schemeObject_toString(string_t * out, schemeObject_t * inobj) {
	stringBuilder_t sb;
	linkedList_t * stack;
	linkedList_add2(&stack, &inobj, schemeObject_t *);
	CHKFAIL(stringBuilder_new(&sb))
	schemeObject_t * cur = inobj;
	if(cur->kind == SCHEME_OBJECT_CONS) {
		CHKFAIL(stringBuilder_append(&sb, "(", 1))
		cur = cur->value.consValue.value;
		if(cur->kind == SCHEME_OBJECT_CONS)
			CHKFAIL(stringBuilder_append(&sb, "(", 1))
	}
	linkedList_pop2(&stack, &cur, schemeObject_t *);
	{ // do-whileにしたかったけど、ダメでした（非効率なコード書けば変形できる）
		L_START:
		if(cur == SCHEME_OBJECT_NILL)
			CHKFAIL(stringBuilder_append(&sb, ")", 1))
		while(cur != SCHEME_OBJECT_NILL) {
			switch(cur->kind) {
				case SCHEME_OBJECT_SYMBOL:
					CHKFAIL(stringBuilder_append2(&sb, &(cur->value.strValue)))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_STRING:
					CHKFAIL(stringBuilder_append(&sb, "\"", 1))
					CHKFAIL(stringBuilder_append2(&sb, &(cur->value.strValue)))
					CHKFAIL(stringBuilder_append(&sb, "\"", 1))
					goto CONTINUE_OUTER_WHILE;
				case SCHEME_OBJECT_NUMBER: {
					char buf[32];
					sprintf(buf, "%d", cur->value.numValue);
					CHKFAIL(stringBuilder_append(&sb, buf, strlen(buf)));
					goto CONTINUE_OUTER_WHILE;
				}
				case SCHEME_OBJECT_CONS: {
					schemeObject_t * ne = cur->value.consValue.next;
					if(ne != SCHEME_OBJECT_NILL && ne->kind != SCHEME_OBJECT_CONS)
						CHKFAIL(stringBuilder_append(&sb, ".", 1))
					linkedList_add2(&stack, &ne, schemeObject_t *);
					cur = cur->value.consValue.value;
					if(cur == SCHEME_OBJECT_NILL) {
						CHKFAIL(stringBuilder_append(&sb, "NIL", 3))
					} else if(cur->kind == SCHEME_OBJECT_CONS)
						CHKFAIL(stringBuilder_append(&sb, "(", 1))
				}
			}
		}
		CONTINUE_OUTER_WHILE:
		if(!linkedList_pop2(&stack, &cur, schemeObject_t *)) goto L_END;
		if(cur != SCHEME_OBJECT_NILL)
			CHKFAIL(stringBuilder_append(&sb, " ", 1))
		goto L_START;
	}
	L_END:
	CHKFAIL(stringBuilder_toString(out, &sb))
	stringBuilder_free(&sb);
	return true;
}