#include "parse.h"
#include "string_t.h"
#include "list.h"
#include "common.h"
#include "gc.h"

typedef struct parseEnv {
	schemeObject_t * head;
	schemeObject_t ** tailnext;
	bool quote;
} parseEnv_t;

error_t
env_append(parseEnv_t * pe, schemeObject_t * so, bool isRefIncrement) {
	schemeObject_t * cell = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (cell == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(schemeObject_new_cons2(cell, so))
	if(isRefIncrement && so != SCHEME_OBJECT_NILL) CHKERROR(gc_ref(&so->gcInfo))
	*pe->tailnext = cell;
	CHKERROR(gc_ref(&cell->gcInfo))
	pe->tailnext = &(cell->value.consValue.next);
	return ERR_SUCCESS;
}

error_t
parse_symbol(schemeObject_t ** so, token_t * ot) {
	if (ot->tokenKind == TOKEN_SYMBOL && (string_equals2(&(ot->value.strValue), "nil", 3) || string_equals2(&(ot->value.strValue), "NIL", 3))) {
		*so = SCHEME_OBJECT_NILL;
		return ERR_SUCCESS;
	}
	*so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
	if (*so == NULL) return ERR_OUT_OF_MEMORY;
	switch (ot->tokenKind) {
	case TOKEN_NUMERIC: return schemeObject_new_number(*so, ot->value.numValue);
	case TOKEN_STRING: return schemeObject_new_string(*so, ot->value.strValue);
	case TOKEN_SYMBOL: return schemeObject_new_symbol(*so, ot->value.strValue);
	default: return ERR_ILLEGAL_STATE; // throw exception.
	}
}

#define PARSE_CHKERROR(val) if(errorReason = (val)) goto L_FAIL;
#define PARSE_POP { \
so = current->head;\
if (!linkedList_pop2(&envStack, NULL, parseEnv_t)) {\
	errorReason = ERR_PARSE_TOO_MUCH_PAREN_CLOSE; goto L_FAIL;\
}\
if (envStack == NULL) goto L_END;\
current = linkedList_get2(envStack, parseEnv_t);\
PARSE_CHKERROR(env_append(current, so, false)) \
}

error_t
parse(schemeObject_t ** out, tokenizer_t * input) {
	token_t ot;
	linkedList_t * envStack = NULL;
	parseEnv_t * current = NULL;
	schemeObject_t * so = NULL;
	error_t errorReason = ERR_SUCCESS;
	while (tokenizer_next(input, &ot)) {
		if (ot.tokenKind == TOKEN_PAREN_OPEN || ot.tokenKind == TOKEN_QUOTE) {
			parseEnv_t newEnv = { NULL, NULL, false };
			PARSE_CHKERROR(linkedList_add2(&envStack, &newEnv, parseEnv_t))
			current = linkedList_get2(envStack, parseEnv_t);
			current->head = NULL;
			current->tailnext = &(current->head);
			if(ot.tokenKind == TOKEN_QUOTE) {
				current->quote = true;
				// quote symbol insertion.
				so = (schemeObject_t *)reallocarray(NULL, 1, sizeof(schemeObject_t));
				if (so == NULL) { errorReason = ERR_OUT_OF_MEMORY; goto L_FAIL; }
				string_t st;
				PARSE_CHKERROR(string_new_deep2(&st, "quote"))
				PARSE_CHKERROR(schemeObject_new_symbol(so, st))
				PARSE_CHKERROR(env_append(current, so, true))
			}
		} else {
			if(current == NULL) // mono symbol.
				if(errorReason = parse_symbol(&so, &ot)) goto L_FAIL; else {CHKERROR(gc_ref(&(so->gcInfo))) goto L_END;}
			if(ot.tokenKind == TOKEN_PAREN_CLOSE) {
				PARSE_POP
				while(current->quote) PARSE_POP // when '( ...
			} else if(errorReason = parse_symbol(&so, &ot)) goto L_FAIL;
			else {
				PARSE_CHKERROR(env_append(current, so, true))
				while(current->quote)PARSE_POP // when 'some
			}
		}
	}
	if (current != NULL && current->quote) {
		so = current->head;
		if (!linkedList_pop2(&envStack, NULL, parseEnv_t)) {
			errorReason = ERR_PARSE_TOO_MUCH_PAREN_CLOSE; goto L_FAIL;
		}
		goto L_END;
	}
	errorReason = ERR_ILLEGAL_STATE;
	goto L_FAIL;

L_END:
	if(envStack != NULL) return ERR_PARSE_TOO_MUCH_PAREN_OPEN; // missing ), too much (((
	if (so == NULL) return ERR_PARSE_NO_INPUT;
	*out = so;
	return ERR_SUCCESS;

L_FAIL:
	return errorReason;
}