#include "schemeObject_t.h"

static schemeObject_t predefined_t = {SCHEME_OBJECT_SYMBOL, 1, .value.strValue={"#t", 2}};
static schemeObject_t predefined_f = {SCHEME_OBJECT_SYMBOL, 1, .value.strValue={"#f", 2}};