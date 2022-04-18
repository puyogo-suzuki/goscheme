#include "gc.h"
#include "schemeObject_t.h"
#include "environment.h"
#include "list.h"
#include "common.h"
void
gcInfo_new(gcInfo_t * out) {
    *out = 0;
}

error_t
gc_deref_schemeObject(struct schemeObject * self) {
    linkedList_t * ll = NULL;
    schemeObject_t * current;
    bool perror_OOMRecoverByFreeing = error_OOMRecoverByFreeing;
    error_OOMRecoverByFreeing = false;
    CHKERROR(linkedList_add2(&ll, &self, schemeObject_t *))
    while(linkedList_pop2(&ll, &current, schemeObject_t *)) {
        while(current != SCHEME_OBJECT_NILL) {
            schemeObject_t * me = current;
            current->gcInfo--;
            if(current->gcInfo < 0) DEBUGBREAK;
            if(current->gcInfo > 0) break;
            current = SCHEME_OBJECT_NILL;
            switch(me->kind) {
                case SCHEME_OBJECT_CONS:
                    CHKERROR(linkedList_add2(&ll, &(me->value.consValue.next), schemeObject_t *))
                    current = me->value.consValue.value;
                    break;
                case SCHEME_OBJECT_NUMBER: break;
                case SCHEME_OBJECT_STRING:
                    string_free(&me->value.strValue);
                    break;
                case SCHEME_OBJECT_SYMBOL:
                    string_free(&me->value.symValue);
                    break;
                case SCHEME_OBJECT_EXTERN_FUNCTION:
                    if(me->value.extFuncValue.environment != NULL)
                        gc_deref_environment(me->value.extFuncValue.environment);
                    break;
            }
            free(me);
        }
    }
    error_OOMRecoverByFreeing = perror_OOMRecoverByFreeing;
    return ERR_SUCCESS;
}
error_t
gc_deref_environment(struct environment * self) {
    environment_t * current = self;
    bool perror_OOMRecoverByFreeing = error_OOMRecoverByFreeing;
    error_OOMRecoverByFreeing = false;
    while(current != NULL) {
        environment_t * me = current;
        current->gcInfo--;
        if(current->gcInfo > 0) break;
        current = current->parent;
        free(me);
    }
    error_OOMRecoverByFreeing = perror_OOMRecoverByFreeing;
    return ERR_SUCCESS;
}
error_t
gc_ref(gcInfo_t * self) {
    *self = *self + 1;
    return ERR_SUCCESS;
}