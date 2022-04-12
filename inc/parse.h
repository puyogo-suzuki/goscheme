#pragma once
#include "token.h"
#include "schemeObject_t.h"
#include "error.h"

error_t
parse(schemeObject_t ** out, tokenizer_t * input);