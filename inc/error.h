#pragma once
#include <stdbool.h>

static bool error_OOMRecoverByFreeing = true;

typedef enum {
	ERR_SUCCESS = 0,
	ERR_OUT_OF_MEMORY,
	ERR_OUT_OF_INDEX,
	ERR_INTEGER_FORMAT,
	ERR_ILLEGAL_STATE,
	ERR_PARSE_TOO_MUCH_PAREN_OPEN,
	ERR_PARSE_TOO_MUCH_PAREN_CLOSE,
	ERR_PARSE_NO_INPUT,
	ERR_EVAL_NOT_FOUND_SYMBOL,
	ERR_EVAL_INVALID_OBJECT_TYPE
} error_t;

#define CHKERROR(act) { error_t result_error = (act); if(result_error != ERR_SUCCESS) return result_error; }