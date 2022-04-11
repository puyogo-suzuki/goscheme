#include "parse.h"
#include "string_t.h"
#include "list.h"
#include "common.h"

typedef struct parseEnv {
	schemeObject_t * head;
	schemeObject_t ** tailnext;
	bool quote;
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
parse_symbol(schemeObject_t ** so, token_t * ot) {
	if (ot->tokenKind == TOKEN_SYMBOL && (string_equals2(&(ot->value.strValue), "nil", 3) || string_equals2(&(ot->value.strValue), "NIL", 3))) {
		*so = SCHEME_OBJECT_NILL;
		return true;
	}
	*so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (*so == NULL) return false;
	switch (ot->tokenKind) {
	case TOKEN_NUMERIC: return schemeObject_new_number(*so, ot->value.numValue);
	case TOKEN_STRING: return schemeObject_new_string(*so, ot->value.strValue);
	case TOKEN_SYMBOL: return schemeObject_new_symbol(*so, ot->value.strValue);
	default: return false; // throw exception.
	}
}

bool
parse(schemeObject_t ** out, tokenizer_t * input) {
	token_t ot;
	linkedList_t * envStack = NULL;
	parseEnv_t * current = NULL;
	schemeObject_t * so = NULL;
	while (tokenizer_next(input, &ot)) {
		if (ot.tokenKind == TOKEN_PAREN_OPEN || ot.tokenKind == TOKEN_QUOTE_PAREN_OPEN) {
			parseEnv_t newEnv = {NULL, NULL, false};
			if (!linkedList_add2(&envStack, &newEnv, parseEnv_t)) goto L_FAIL;
			current = linkedList_get2(envStack, parseEnv_t);
			current->head = NULL;
			current->tailnext = &(current->head); // newEnvでやると，関数抜けたときnewEnvは解放されてしまいます．
			if(ot.tokenKind == TOKEN_QUOTE_PAREN_OPEN) {
				current->quote = true;
				// quote symbol insertion.
				so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
				if (so == NULL) goto L_FAIL;
				string_t st;
				string_new_deep2(&st, "quote");
				if(!schemeObject_new_symbol(so, st)) goto L_FAIL;
				if(!env_append(current, so)) goto L_FAIL;
				// create new list.
				if (!linkedList_add2(&envStack, &newEnv, parseEnv_t)) goto L_FAIL;
				current = linkedList_get2(envStack, parseEnv_t);
				current->head = NULL;
				current->tailnext = &(current->head); // newEnvでやると，関数抜けたときnewEnvは解放されてしまいます．
			}
		} else {
			if (current == NULL) { // mono symbol.
				if(!parse_symbol(&so, &ot)) goto L_FAIL;// too much ))).
				goto L_END;
			}
			if(ot.tokenKind == TOKEN_PAREN_CLOSE) { 
				so = current->head;
				linkedList_pop2(&envStack, NULL, parseEnv_t);
				if (envStack == NULL) goto L_END;
				current = linkedList_get2(envStack, parseEnv_t);
				if(current->quote) { // when '(
					if (!env_append(current, so)) goto L_FAIL;
					so = current->head;
					if(!linkedList_pop2(&envStack, NULL, parseEnv_t)) goto L_FAIL; // too much )))).
					if (envStack == NULL) goto L_END;
					current = linkedList_get2(envStack, parseEnv_t);
				}
			} else {
				so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
				if (so == NULL) goto L_FAIL;
				parse_symbol(&so, &ot);
			}
			if (!env_append(current, so)) goto L_FAIL;
		}
	}
	goto L_FAIL;

L_END:
	if(envStack != NULL) return false; // missing ), too much (((
	if (so == NULL) return false;
	*out = so;
	return true;

L_FAIL:
	return false;
}