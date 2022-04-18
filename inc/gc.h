#include <stdint.h>
#include "error.h"
typedef int32_t gcInfo_t;

struct schemeObject;
struct environment;

void
gcInfo_new(gcInfo_t * out);
error_t
gc_deref_schemeObject(struct schemeObject * self);
error_t
gc_deref_environment(struct environment * self);
error_t
gc_ref(gcInfo_t * self);