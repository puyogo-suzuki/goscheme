#include <stdint.h>
#include "error.h"
typedef int32_t gcInfo_t;

struct schemeObject;
struct environment;

void
gcInfo_new(gcInfo_t * out);
gserror_t
gc_deref_schemeObject(struct schemeObject * self);
gserror_t
gc_deref_environment(struct environment * self);
gserror_t
gc_ref(gcInfo_t * self);