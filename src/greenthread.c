#include "greenthread.h"
#include "machine.h"
#include "schemeObject_t.h"
#include "io.h"

machine_t *
scheduler_getNext(scheduler_t * schd) {
	machine_t * m = NULL;
	linkedListAppend_pop2(&(schd->machines), &m, machine_t *);
	return m;
}

gserror_t
scheduler_new(scheduler_t * outval) {
	linkedListAppend_init(&(outval->machines), LINKEDLIST_TERMINATOR);
	return ERR_SUCCESS;
}

gserror_t
runner_new(runner_t * outval, scheduler_t * paren) {
	outval->parent = paren;
	outval->garbage = NULL;
	return ERR_SUCCESS;
}

#if _MSC_VER
gserror_t
runner_new_spawn(runner_t * outval, scheduler_t * paren, LPTHREAD_START_ROUTINE routine, LPVOID param, LPDWORD id) {
	outval->parent = paren;
	outval->garbage = NULL;
	outval->hThread = CreateThread(NULL, 0, routine, param, 0, id);
	return outval->hThread == NULL ? ERR_ILLEGAL_STATE : ERR_SUCCESS;
}
#elif _SYSV
gserror_t
runner_new_spawn(runner_t * outval, scheduler_t * paren, void * routine (void *), void * param) {
	outval->parent = paren;
	if(pthread_create(&(outval->thread), NULL, routine, param) != 0) return ERR_ILLEGAL_STATE;
	return ERR_SUCCESS;
}
#elif _ESP
gserror_t
runner_new_spawn(runner_t * outval, scheduler_t * paren, void routine (void *), void * param) {
	outval->parent = paren;
	if(xTaskCreatePinnedToCore(routine, "RUNNER Thread", 8192, param, 1, &(outval->thread), 1) == pdFAIL) return ERR_ILLEGAL_STATE;
	return ERR_SUCCESS;	
}
#endif

void
runner_terminate(runner_t * self, machine_t * m) {
#if _ESP
	TaskHandle_t th = self->thread;
	free(m);
	free(self);
	vTaskDelete(th);
#else
	while (true) {
		machine_t * getNext = scheduler_getNext(self->parent);
		if (getNext == NULL) {
#if _MSC_VER
			Sleep(500);
#elif _SYSV
			usleep(500 * 1000);
#endif
			continue;
		}
		getNext->runner = self;
		self->garbage = m;
#if _MSC_VER
		SwitchToFiber(getNext->fiber);
#elif _SYSV
		setcontext(&(getNext->uc));
#endif
		// this means return;
	}
#endif
}

#if _MSC_VER || _SYSV
void
greenthread_free_machine(machine_t ** machine) {
	if(*machine == NULL) return;
#if _MSC_VER
	DeleteFiber((*machine)->fiber);
#elif _SYSV
	free((*machine)->uc.uc_stack.ss_sp);
#endif
	free(*machine);
	*machine = NULL;
}

typedef struct starter { machine_t * machine; schemeObject_t * func;  } starter_t;
#if _MSC_VER
void
greenthread_starter(LPVOID arg) {
#elif _SYSV
void
greenthread_starter(void * arg) {
#endif
	starter_t s = *(starter_t *)arg;
	free(arg);
	schemeObject_t * obj = NULL;
	string_t str = {NULL};
	greenthread_free_machine(&(s.machine->runner->garbage));
	if (machine_evalforce(s.machine, s.machine->env, s.func, &obj) == ERR_SUCCESS) {
		if (schemeObject_toString(&str, obj) == ERR_SUCCESS)
			string_writeLine(stdout, &str);
		else
			errorOut("ERROR", "greenthread_starter", "schemeObject_toString failure.");
		gc_deref_schemeObject(obj);
	}
	gc_deref_schemeObject(s.func);
	machine_t * getNext = scheduler_getNext(s.machine->runner->parent);
	if (getNext == s.machine)
		runner_terminate(s.machine->runner, s.machine);
	else {
		getNext->runner = s.machine->runner;
		s.machine->runner->garbage = s.machine;
#if _MSC_VER
		SwitchToFiber(getNext->fiber);
#else
		setcontext(&(getNext->uc));
#endif
		// this means return;
	}
}
#elif _ESP
typedef struct starter { machine_t * machine; schemeObject_t * func;  } starter_t;
void
greenthread_starter(starter_t * arg) {
	schemeObject_t * obj = NULL;
	string_t str = {NULL};
	if (machine_evalforce(arg->machine, arg->machine->env, arg->func, &obj) == ERR_SUCCESS) {
		if (schemeObject_toString(&str, obj) == ERR_SUCCESS)
			string_writeLine(stdout, &str);
		else
			errorOut("ERROR", "greenthread_starter", "schemeObject_toString failure.");
		gc_deref_schemeObject(obj);
	}
	gc_deref_schemeObject(arg->func);
	machine_t * m = arg->machine;
	free(arg);
	runner_terminate(m->runner, m);
}
#endif

gserror_t
greenthread_init(machine_t * self, schemeObject_t * function, scheduler_t * registerTo) {
#if _MSC_VER
	starter_t * v = (starter_t *)reallocarray(NULL, 1, sizeof(starter_t));
	if (v == NULL) return ERR_OUT_OF_MEMORY;
	v->func = function;
	v->machine = self;
	self->fiber = CreateFiber(0, greenthread_starter, v);
	CHKERROR(linkedListAppend_append2(&(registerTo->machines), &self, machine_t *))
#elif _SYSV
	starter_t * v = (starter_t *)reallocarray(NULL, 1, sizeof(starter_t));
	if (v == NULL) return ERR_OUT_OF_MEMORY;
	v->func = function;
	v->machine = self;
	getcontext(&(self->uc));
    self->uc.uc_link          = NULL; // とりあえずNULL
    self->uc.uc_stack.ss_sp   = (void *)reallocarray(NULL, 1, sizeof(char) * 2 * 1024 * 1024);
    self->uc.uc_stack.ss_size = sizeof(char) * 2 * 1024 * 1024;

	makecontext(&(self->uc), (void (*)(void))greenthread_starter, 1, (void*)v);
	CHKERROR(linkedListAppend_append2(&(registerTo->machines), &self, machine_t *))
#elif _ESP
	starter_t * v = (starter_t *)reallocarray(NULL, 1, sizeof(starter_t));
	if (v == NULL) return ERR_OUT_OF_MEMORY;
	v->func = function;
	v->machine = self;
	runner_t * runner = (runner_t *)reallocarray(NULL, 1, sizeof(runner_t));
	if (runner == NULL) return ERR_OUT_OF_MEMORY;
	self->runner = runner;
	CHKERROR(runner_new_spawn(runner, registerTo, (void (*)(void *))greenthread_starter, (void *)v))
#endif
	return ERR_SUCCESS;
}

gserror_t
greenthread_sleep(machine_t * self, int milliSeconds) {
	int I = milliSeconds / 100;
	for (int i = 0; i < I; ++i) {
		CHKERROR(greenthread_yield(self))
#if _MSC_VER
		Sleep(100);
#else
		usleep(100*1000);
#endif
	}
	return ERR_SUCCESS;
}

gserror_t
greenthread_yield(struct machine * self) {
	if (self->runner == NULL) return ERR_SUCCESS; // NOT MULTITHREAD.
#if _MSC_VER
	machine_t * getNext = scheduler_getNext(self->runner->parent);
	if (getNext == NULL)
		Sleep(50);
	else {
		runner_t * ru = self->runner;
		CHKERROR(linkedListAppend_append2(&(self->runner->parent->machines), &self, machine_t *))
		self->runner = NULL;
		getNext->runner = ru;
		SwitchToFiber(getNext->fiber);
#pragma warning(disable:6011)
		greenthread_free_machine(&(self->runner->garbage));
#pragma warning(default:6011)
	}
#elif _SYSV
	machine_t * getNext = scheduler_getNext(self->runner->parent);
	if (getNext == NULL)
		usleep(50*1000);
	else {
		runner_t * ru = self->runner;
		CHKERROR(linkedListAppend_append2(&(self->runner->parent->machines), &self, machine_t *))
		self->runner = NULL;
		getNext->runner = ru;
		swapcontext(&(self->uc), &(getNext->uc));
	}
#endif
	return ERR_SUCCESS;
}

gserror_t
greenthread_spawn(struct machine * self, struct schemeObject * inval) {
	machine_t * newmachine = (machine_t *)reallocarray(NULL, 1, sizeof(machine_t));
	if (newmachine == NULL) return ERR_OUT_OF_MEMORY;
	CHKERROR(gc_ref(&(inval->gcInfo)))
	CHKERROR(machine_new(newmachine, self->env))
	CHKERROR(greenthread_init(newmachine, inval, self->runner->parent))
	return ERR_SUCCESS;
}

gserror_t
greenthread_mailbox_receive(struct machine * self, struct schemeObject ** outobj) {
	return ERR_SUCCESS;
}

gserror_t
greenthread_mailbox_send(struct machine * self, struct schemeObject * inobj) {
	return ERR_SUCCESS;
}
