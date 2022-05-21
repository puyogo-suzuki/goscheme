#include <stdio.h>
#include "token.h"
#include "io.h"
#include "parse.h"
#include "machine.h"
#include "gc.h"

#if _MSC_VER
typedef struct repl { HANDLE mutex, mutex1; machine_t * self; schemeObject_t ** parsedVal; } repl_t;

DWORD WINAPI
repl_thread(LPVOID p) {
    repl_t param = *(repl_t *)p;
    gserror_t evalerrno;
    param.self->fiber = ConvertThreadToFiber(NULL);
    while (TRUE) {
        schemeObject_t * res;
        string_t str;
        if (WaitForSingleObject(param.mutex, 0) == WAIT_OBJECT_0) {
            if (*(param.parsedVal) == NULL) {
                return 0;
            }
            if (evalerrno = machine_evalforce(param.self, param.self->env, *(param.parsedVal), &res)) printf("eval fail : %d\n", evalerrno);
            else {
                schemeObject_toString(&str, res);
                string_writeLine(stdout, &str);
                string_free(&str);
                gc_deref_schemeObject(res);
            }
            gc_deref_schemeObject(*(param.parsedVal));
            *(param.parsedVal) = NULL;
            SetEvent(param.mutex1);
        } else {
            greenthread_yield(param.self);
        }
    }
    return 0;
}

int main(void) {
    environment_t * env = (environment_t *)reallocarray(NULL, 1, sizeof(environment_t));
    machine_t * vm = (machine_t *)reallocarray(NULL, 1, sizeof(machine_t));
    runner_t * runner = (runner_t *)reallocarray(NULL, 1, sizeof(runner_t));
    scheduler_t * schd = (scheduler_t *)reallocarray(NULL, 1, sizeof(scheduler_t));
    schemeObject_t * so = NULL;
    environment_new_global(env);
    machine_new(vm, env);
    scheduler_new(schd);
    repl_t param = { .mutex = CreateEvent(0, FALSE, FALSE, NULL), .mutex1 = CreateEvent(0, FALSE, FALSE, NULL), .self = vm, .parsedVal = &so};
    if (param.mutex == NULL || param.mutex1 == NULL) return 1;
    DWORD v;
    vm->runner = runner;
    runner_new_spawn(runner, schd, repl_thread, &param, &v);
    while (1) {
        char buf[1024];
        fgets(buf, 1023, stdin);
        string_t str;
        string_new_shallow2(&str,  buf);
        tokenizer_t t = tokenizer_new(str);
        gserror_t evalerrno = ERR_SUCCESS;
        if (parse(&so, &t)) printf("parse fail\n");
        else SetEvent(param.mutex);
        WaitForSingleObject(param.mutex1, INFINITE);
    }
    printf("end\n");
    return 0;
}
#else
int main(void) {
    machine_t vm;
    machine_new(&vm, NULL);
    while (1) {
        char buf[1024];
        fgets(buf, 1023, stdin);
        string_t str;
        string_new_shallow2(&str, buf);
        tokenizer_t t = tokenizer_new(str);
        schemeObject_t * so, * res;
        gserror_t evalerrno = ERR_SUCCESS;
        if (parse(&so, &t)) printf("parse fail\n");
        else {
            if (evalerrno = machine_evalforce(&vm, vm.env, so, &res)) printf("eval fail : %d\n", evalerrno);
            else {
                schemeObject_toString(&str, res);
                string_writeLine(stdout, &str);
                string_free(&str);
                gc_deref_schemeObject(res);
            }
            gc_deref_schemeObject(so);
        }
    }
    printf("end\n");
    return 0;
}
#endif

