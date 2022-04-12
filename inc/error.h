#pragma once

typedef enum {
	ERR_SUCCESS = 0,
	ERR_OUT_OF_MEMORY,
	ERR_OUT_OF_INDEX,
	ERR_INTEGER_FORMAT,
	ERR_ILLEGAL_STATE,
	ERR_PARSE_TOO_MUCH_PAREN_OPEN,
	ERR_PARSE_TOO_MUCH_PAREN_CLOSE,
	ERR_PARSE_NO_INPUT
} error_t;

#define CHKERROR(act) { error_t result_error = (act); if(result_error != ERR_SUCCESS) return result_error; }