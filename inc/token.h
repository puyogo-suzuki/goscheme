#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "string_t.h"

typedef enum token_kind {
    TOKEN_PAREN_OPEN,
    TOKEN_QUOTE,
    TOKEN_PAREN_CLOSE,
    TOKEN_SYMBOL,
    TOKEN_STRING,
    TOKEN_NUMERIC
} token_kind_t;

typedef struct token {
    token_kind_t tokenKind;
    union {
        int32_t numValue;
        string_t strValue;
    } value;
} token_t;

typedef struct tokenizer {
    size_t position;
    string_t str;
} tokenizer_t;


tokenizer_t
tokenizer_new(string_t str);

bool
tokenizer_next(tokenizer_t * self, token_t * out_token);

error_t
token_toString(token_t * self, string_t * out);
