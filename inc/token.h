#include <stdint.h>
#include <stdbool.h>
#include "string_t.h"

typedef int token_kind_t;
static const token_kind_t TOKEN_PAREN_OPEN = 0;
static const token_kind_t TOKEN_PAREN_CLOSE = 1;
static const token_kind_t TOKEN_SYMBOL = 2;
static const token_kind_t TOKEN_STRING = 3;
static const token_kind_t TOKEN_NUMERIC = 4;

typedef struct token {
    token_kind_t tokenKind;
    union {
        int32_t numValue;
        string_t strValue;
    } value;
} token_t;

typedef struct tokenizer {
    int32_t position;
    string_t str;
} tokenizer_t;


tokenizer_t
tokenizer_new(string_t str);

bool
tokenizer_next(tokenizer_t * self, token_t * out_token);