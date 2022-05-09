#if _ESP
#include "esp32peripheral.h"
#include "builtinfuncs.h"
#include "gc.h"
#include "machine.h"
#include "common.h"
#include "schemeObject_predefined_object.h"

ONE_ARGUMENT_FUNC(schemefunc_esp_lcdDbg, "lcd-dbg", { \
    string_t outstr = {NULL}; \
    CHKERROR(schemeObject_toString(&outstr, arg0)) \
    lcdPrint(outstr.buffer); \
    string_free(&outstr); \
})

ONE_ARGUMENT_FUNC(schemefunc_esp_lcdPrint, "lcd-print", { \
    if(arg0 != SCHEME_OBJECT_NILL && arg0->kind == SCHEME_OBJECT_STRING) { \
        lcdPrint(arg0->value.strValue.buffer); \
    } else { \
        errorOut("ERROR", "lcd-print", "String is needed."); \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        return ERR_EVAL_INVALID_OBJECT_TYPE;  \
    }\
})


#define SCHEMEFUNC_HEAD(name_str, a, machine, env, val, count) { \
    schemeObject_t * cur = val; \
    if(schemeObject_length(val) != count) { \
		errorOut("ERROR", name_str, "Invalid arity."); \
		return ERR_EVAL_ARGUMENT_MISMATCH; \
    } \
    CHKERROR(gc_ref(&(val->gcInfo))) \
    for(int i = 0; i < count; ++i) { \
        schemeObject_t * prev = cur; \
        schemeObject_t * tmp = NULL; \
        CHKERROR(schemeObject_car(cur, &tmp)) \
        CHKERROR(machine_evalforce(machine, env, tmp, &a[i])) \
        CHKERROR(gc_deref_schemeObject(tmp)) \
        CHKERROR(schemeObject_cdr(prev, &cur)) \
        CHKERROR(gc_deref_schemeObject(prev)) \
    } \
}
gserror_t
schemefunc_esp_lcdDrawPixel(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
    schemeObject_t * args[3];
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    SCHEMEFUNC_HEAD("lcd-drawpixel", args, self, env, val, 3)
    bool argCheck = true;
    for(int i = 0; i < 3; ++i) argCheck &= args[i] != SCHEME_OBJECT_NILL && args[i]->kind == SCHEME_OBJECT_NUMBER;
    if(!argCheck) {
        errorOut("ERROR", "lcd-drawpixel", "Invalid argments. Needed Number, Number, Number.");
        for(int i = 0; i < 3; ++i) CHKERROR(gc_deref_schemeObject(args[i]))
        return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    lcdDrawPixel(args[0]->value.numValue, args[1]->value.numValue, args[2]->value.numValue);
    for(int i = 0; i < 3; ++i) CHKERROR(gc_deref_schemeObject(args[i]))
    return ERR_SUCCESS;
}
gserror_t
schemefunc_esp_lcdDrawLine(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
    schemeObject_t * args[5];
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    SCHEMEFUNC_HEAD("lcd-drawline", args, self, env, val, 5)
    bool argCheck = true;
    for(int i = 0; i < 5; ++i) argCheck &= args[i] != SCHEME_OBJECT_NILL && args[i]->kind == SCHEME_OBJECT_NUMBER;
    if(!argCheck) {
        errorOut("ERROR", "lcd-drawline", "Invalid argments. Needed Number, Number, Number, Number, Number.");
        for(int i = 0; i < 5; ++i) CHKERROR(gc_deref_schemeObject(args[i]))
        return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    lcdDrawLine(args[0]->value.numValue, args[1]->value.numValue, args[2]->value.numValue, args[3]->value.numValue, args[4]->value.numValue);
    for(int i = 0; i < 5; ++i) CHKERROR(gc_deref_schemeObject(args[i]))
    return ERR_SUCCESS;
}
gserror_t
schemefunc_esp_lcdFillRect(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
    schemeObject_t * args[5];
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    SCHEMEFUNC_HEAD("lcd-fillrect", args, self, env, val, 5)
    bool argCheck = true;
    for(int i = 0; i < 5; ++i) argCheck &= args[i] != SCHEME_OBJECT_NILL && args[i]->kind == SCHEME_OBJECT_NUMBER;
    if(!argCheck) {
        errorOut("ERROR", "lcd-fillrect", "Invalid argments. Needed Number, Number, Number, Number, Number.");
        for(int i = 0; i < 5; ++i) CHKERROR(gc_deref_schemeObject(args[i]))
        return ERR_EVAL_INVALID_OBJECT_TYPE;
    }
    lcdFillRect(args[0]->value.numValue, args[1]->value.numValue, args[2]->value.numValue, args[3]->value.numValue, args[4]->value.numValue);
    for(int i = 0; i < 5; ++i) CHKERROR(gc_deref_schemeObject(args[i]))
    return ERR_SUCCESS;
}

ONE_ARGUMENT_FUNC(schemefunc_esp_lcdFillScreen, "lcd-fillscreen", { \
    if(arg0 != SCHEME_OBJECT_NILL && arg0->kind == SCHEME_OBJECT_NUMBER) { \
        lcdFillScreen(arg0->value.numValue); \
    } else { \
        errorOut("ERROR", "lcd-fillscreen", "Number is needed."); \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        return ERR_EVAL_INVALID_OBJECT_TYPE;  \
    }\
})
TWO_ARGUMENT_FUNC(schemefunc_esp_lcdSetCursor, "lcd-setcursor", { \
    if(arg0 != SCHEME_OBJECT_NILL && arg0->kind == SCHEME_OBJECT_NUMBER
        && arg1 != SCHEME_OBJECT_NILL && arg1->kind == SCHEME_OBJECT_NUMBER) { \
        lcdSetCursor(arg0->value.numValue, arg1->value.numValue); \
    } else { \
        errorOut("ERROR", "lcd-setcursor", "Invalid arguments. Needed Number, Number."); \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        CHKERROR(gc_deref_schemeObject(arg1)) \
        return ERR_EVAL_INVALID_OBJECT_TYPE;  \
    }\
}, true)
TWO_ARGUMENT_FUNC(schemefunc_esp_lcdSetTextColor, "lcd-settextcolor", { \
    if(arg0 != SCHEME_OBJECT_NILL && arg0->kind == SCHEME_OBJECT_NUMBER
        && arg1 != SCHEME_OBJECT_NILL && arg1->kind == SCHEME_OBJECT_NUMBER) { \
        lcdSetTextColor(arg0->value.numValue, arg1->value.numValue); \
    } else { \
        errorOut("ERROR", "lcd-settextcolor", "Invalid arguments. Needed Number, Number."); \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        CHKERROR(gc_deref_schemeObject(arg1)) \
        return ERR_EVAL_INVALID_OBJECT_TYPE;  \
    }\
}, true)
ONE_ARGUMENT_FUNC(schemefunc_esp_lcdSetTextSize, "lcd-settextsize", { \
    if(arg0 != SCHEME_OBJECT_NILL && arg0->kind == SCHEME_OBJECT_NUMBER) { \
        lcdSetTextSize(arg0->value.numValue); \
    } else { \
        errorOut("ERROR", "lcd-settextsize", "Number is needed."); \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        return ERR_EVAL_INVALID_OBJECT_TYPE;  \
    }\
})
ONE_ARGUMENT_FUNC(schemefunc_esp_M5Sleep, "sleep", { \
    if(arg0 != SCHEME_OBJECT_NILL && arg0->kind == SCHEME_OBJECT_NUMBER) { \
        M5Sleep(arg0->value.numValue); \
    } else { \
        errorOut("ERROR", "sleep", "Number is needed."); \
        CHKERROR(gc_deref_schemeObject(arg0)) \
        return ERR_EVAL_INVALID_OBJECT_TYPE;  \
    }\
})
gserror_t
schemefunc_esp_M5Update(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
    out->kind = EVALUATIONRESULT_EVALUATED;
    out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    if(val != SCHEME_OBJECT_NILL) {
        errorOut("ERROR", "M5-update", "No argument required."); \
        return ERR_EVAL_ARGUMENT_MISMATCH;
    }
    M5Update();
    return ERR_SUCCESS;
}
gserror_t
schemefunc_esp_buttonAPressp(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    if(val != SCHEME_OBJECT_NILL) {
        errorOut("ERROR", "buttonA-press?", "No argument required."); \
        return ERR_EVAL_ARGUMENT_MISMATCH;
    }
	out->value.evaluatedValue = buttonAIsPressed() ? &predefined_t : &predefined_f;
    CHKERROR(gc_ref(&(out->value.evaluatedValue->gcInfo)))
    return ERR_SUCCESS;
}
gserror_t
schemefunc_esp_buttonBPressp(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    if(val != SCHEME_OBJECT_NILL) {
        errorOut("ERROR", "buttonB-press?", "No argument required."); \
        return ERR_EVAL_ARGUMENT_MISMATCH;
    }
	out->value.evaluatedValue = buttonBIsPressed() ? &predefined_t : &predefined_f;
    CHKERROR(gc_ref(&(out->value.evaluatedValue->gcInfo)))
    return ERR_SUCCESS;
}
gserror_t
schemefunc_esp_buttonCPressp(machine_t * self, environment_t * env, schemeObject_t * val, evaluationResult_t * out) {
	out->kind = EVALUATIONRESULT_EVALUATED;
	out->value.evaluatedValue = SCHEME_OBJECT_NILL;
    if(val != SCHEME_OBJECT_NILL) {
        errorOut("ERROR", "buttonC-press?", "No argument required."); \
        return ERR_EVAL_ARGUMENT_MISMATCH;
    }
	out->value.evaluatedValue = buttonCIsPressed() ? &predefined_t : &predefined_f;
    CHKERROR(gc_ref(&(out->value.evaluatedValue->gcInfo)))
    return ERR_SUCCESS;
}
#endif