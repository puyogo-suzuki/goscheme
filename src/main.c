#include <stdio.h>
#include "token.h"
#include "io.h"
#include "parse.h"
#include "machine.h"
#include "gc.h"

#if _MSC_VER || _SYSV
#if _MSC_VER
typedef struct repl { HANDLE ev, ev1; machine_t * self; schemeObject_t * parsedVal; } repl_t;

DWORD WINAPI
repl_thread(repl_t * param) {
#elif _SYSV
#include <pthread.h>
#include <semaphore.h>
typedef struct repl { sem_t sem, sem1; machine_t * self; schemeObject_t * parsedVal; } repl_t;

void *
repl_thread(repl_t * param) {
#endif
    gserror_t evalerrno;
#if _MSC_VER
    param->self->fiber = ConvertThreadToFiber(NULL);
    while (TRUE) {
#elif _SYSV
    while(1) {
#endif
        schemeObject_t * res;
        string_t str;
#if _MSC_VER
        if (WaitForSingleObject(param->ev, 0) == WAIT_OBJECT_0) {
#elif _SYSV
        if (sem_trywait(&(param->sem)) == 0) {
#endif
            if (param->parsedVal == NULL) {
                return 0;
            }
            if (evalerrno = machine_evalforce(param->self, param->self->env, param->parsedVal, &res)) printf("eval fail : %d\n", evalerrno);
            else {
                schemeObject_toString(&str, res);
                string_writeLine(stdout, &str);
                string_free(&str);
                gc_deref_schemeObject(res);
            }
            gc_deref_schemeObject(param->parsedVal);
            param->parsedVal = NULL;
#if _MSC_VER
            SetEvent(param->ev1);
#elif _SYSV
            sem_post(&(param->sem1));
#endif
        } else
            greenthread_yield(param->self);
    }
    return 0;
}

int main(void) {
    environment_t * env = (environment_t *)reallocarray(NULL, 1, sizeof(environment_t));
    machine_t * vm = (machine_t *)reallocarray(NULL, 1, sizeof(machine_t));
    runner_t * runner = (runner_t *)reallocarray(NULL, 1, sizeof(runner_t));
    scheduler_t * schd = (scheduler_t *)reallocarray(NULL, 1, sizeof(scheduler_t));
    environment_new_global(env);
    machine_new(vm, env);
    scheduler_new(schd);
    repl_t * param = (repl_t *) reallocarray(NULL, 1, sizeof(repl_t));
    param->self = vm;
    param->parsedVal = NULL;
#if _MSC_VER
    param->ev = CreateEvent(0, FALSE, FALSE, NULL);
    param->ev1 = CreateEvent(0, FALSE, FALSE, NULL);
    if (param->ev == NULL || param->ev1 == NULL) return 1;
#elif _SYSV
    if (sem_init(&(param->sem), 0, 0) || sem_init(&(param->sem1), 0, 0)) return 1;
#endif
    vm->runner = runner;
#if _MSC_VER
    DWORD v;
    runner_new_spawn(runner, schd, repl_thread, param, &v);
#elif _SYSV
    runner_new_spawn(runner, schd, (void * (*) (void*))repl_thread, param);
#endif
    while (1) {
        char buf[1024];
        fgets(buf, 1023, stdin);
        string_t str;
        string_new_shallow2(&str,  buf);
        tokenizer_t t = tokenizer_new(str);
        if (parse(&param->parsedVal, &t)) printf("parse fail\n");
        else {
#if _MSC_VER
            SetEvent(param->ev);
            WaitForSingleObject(param->ev1, INFINITE);
#elif _SYSV
            sem_post(&(param->sem));
            sem_wait(&(param->sem1));
#endif
        }
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

