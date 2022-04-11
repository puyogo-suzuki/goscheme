#include "parse.h"
#include "string_t.h"
#include "list.h"
#include "common.h"

typedef struct parseEnv {
	schemeObject_t * head;
	schemeObject_t ** tailnext;
} parseEnv_t;

bool
env_append(parseEnv_t * pe, schemeObject_t * so) {
	schemeObject_t * cell = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (cell == NULL) return false;
	schemeObject_new_cons2(cell, so);
	*pe->tailnext = cell;
	pe->tailnext = &(cell->value.consValue.next);
	return true;
}

bool
parse(schemeObject_t ** out, tokenizer_t * input) {
	token_t ot;
	linkedList_t * envStack = NULL;
	parseEnv_t * current = NULL;
	schemeObject_t * so = NULL;
	while (tokenizer_next(input, &ot)) {
		if (ot.tokenKind == TOKEN_PAREN_OPEN) {
			parseEnv_t newEnv;
			if (!linkedList_add2(&envStack, &newEnv, parseEnv_t)) goto L_FAIL;
			current = linkedList_get2(envStack, parseEnv_t);
			current->head = NULL;
			current->tailnext = &(current->head); // newEnvでやると，関数抜けたときnewEnvは解放されてしまいます．
		} else {
			if (current == NULL) { // mono symbol.
				so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
				if (so == NULL) goto L_FAIL;
				switch (ot.tokenKind) {
				case TOKEN_NUMERIC: schemeObject_new_number(so, ot.value.numValue); break;
				case TOKEN_STRING: schemeObject_new_string(so, ot.value.strValue); break;
				case TOKEN_SYMBOL: schemeObject_new_symbol(so, ot.value.strValue); break;
				default: DEBUGBREAK; break; // throw exception.
				}
				goto L_END;
			}
			so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
			if (so == NULL) goto L_FAIL;
			switch (ot.tokenKind) {
			case TOKEN_PAREN_CLOSE: schemeObject_new_nil(so); break;
			case TOKEN_NUMERIC: schemeObject_new_number(so, ot.value.numValue); break;
			case TOKEN_STRING: schemeObject_new_string(so, ot.value.strValue); break;
			case TOKEN_SYMBOL: schemeObject_new_symbol(so, ot.value.strValue); break;
			default: break; // make gcc silent!
			}
			if (ot.tokenKind == TOKEN_PAREN_CLOSE) {
				if (!env_append(current, so)) goto L_FAIL;
				so = current->head;
				linkedList_pop2(&envStack, NULL, parseEnv_t);
				if (envStack == NULL) goto L_END;
				current = linkedList_get2(envStack, parseEnv_t);
			}
			if (!env_append(current, so)) goto L_FAIL;
		}
	}
	goto L_FAIL;

L_END:
	if (so == NULL) return false;
	*out = so;
	return true;

L_FAIL:
	return false;
}