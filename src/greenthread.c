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
#if _MSC_VER
	outval->toDeleteFiber = NULL;
#endif
	return ERR_SUCCESS;
}

#if _MSC_VER
gserror_t
runner_new_spawn(runner_t * outval, scheduler_t * paren, LPTHREAD_START_ROUTINE routine, LPVOID param, LPDWORD id) {
	outval->parent = paren;
	outval->toDeleteFiber = NULL;
	outval->hThread = CreateThread(NULL, 0, routine, param, 0, id);
	return ERR_SUCCESS;
}
#endif

void
runner_terminate(runner_t * self) {
	while (true) {
#if _MSC_VER
		machine_t * getNext = scheduler_getNext(self->parent);
		if (getNext == NULL) {
			Sleep(500);
			continue;
		}
		getNext->runner = self;
		self->toDeleteFiber = GetCurrentFiber();
		SwitchToFiber(getNext->fiber);
		// this means return;
#endif
	}
}

typedef struct starter { machine_t * machine; schemeObject_t * func;  } starter_t;
#if _MSC_VER
void
greenthread_starter(LPVOID arg) {
	starter_t s = *(starter_t *)arg;
	free(arg);
	schemeObject_t * obj = NULL;
	string_t str = {NULL};
	if (s.machine->runner->toDeleteFiber != NULL) {
		DeleteFiber(s.machine->runner->toDeleteFiber);
		s.machine->runner->toDeleteFiber = NULL;
	}
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
		runner_terminate(s.machine->runner);
	else {
		getNext->runner = s.machine->runner;
		free(s.machine);
		getNext->runner->toDeleteFiber = GetCurrentFiber();
		SwitchToFiber(getNext->fiber);
		// this means return;
	}
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
#if _MSC_VER
		Sleep(50);
#else
		usleep(100 * 1000);
#endif
	else {
		runner_t * ru = self->runner;
		CHKERROR(linkedListAppend_append2(&(self->runner->parent->machines), &self, machine_t *))
		self->runner = NULL;
		getNext->runner = ru;
#if _MSC_VER
		SwitchToFiber(getNext->fiber);
#pragma warning(disable:6011)
		if (self->runner->toDeleteFiber != NULL) {
			DeleteFiber(self->runner->toDeleteFiber);
			self->runner->toDeleteFiber = NULL;
		}
#pragma warning(default:6011)
#endif
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
