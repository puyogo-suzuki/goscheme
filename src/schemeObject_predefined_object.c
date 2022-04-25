#include "schemeObject_predefined_object.h"

schemeObject_t predefined_t = {SCHEME_OBJECT_SYMBOL, 1, .value.strValue={"#t", 2}};
schemeObject_t predefined_f = {SCHEME_OBJECT_SYMBOL, 1, .value.strValue={"#f", 2}};