#pragma once
#include "error.h"
#include "list.h"
#if _MSC_VER
#include <Windows.h>
#else
#include <unistd.h>
#endif

struct schemeObject;
struct machine;
struct scheduler;

typedef struct runner {
	struct scheduler * parent;
#if _MSC_VER
	HANDLE hThread;
	LPVOID toDeleteFiber;
#endif
} runner_t;

typedef struct scheduler {
	linkedList_t runners;
	linkedListAppend_t machines;
} scheduler_t;

gserror_t
scheduler_new(scheduler_t * outval);

gserror_t
runner_new(runner_t * outval, scheduler_t * paren);

#if _MSC_VER
gserror_t
runner_new_spawn(runner_t * outval, scheduler_t * paren, LPTHREAD_START_ROUTINE routine, LPVOID param, LPDWORD id);
#endif

gserror_t
greenthread_sleep(struct machine * self, int milliSeconds);

gserror_t
greenthread_yield(struct machine * self);

gserror_t
greenthread_spawn(struct machine * self, struct schemeObject * inval);

gserror_t
greenthread_mailbox_receive(struct machine * self, struct schemeObject ** outobj);

gserror_t
greenthread_mailbox_send(struct machine * self, struct schemeObject * inobj);