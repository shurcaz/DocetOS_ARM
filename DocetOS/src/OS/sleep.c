#define OS_INTERNAL
#include "OS/sleep.h"
#include "OS/os.h"
#include "OS/Lib/min_heap.h"

/* comparison function and initialisation of sleep list heap */
static inline int_fast8_t wake_time_comparator(void * a, void * b) {
	uint32_t wake_time_a = (uint32_t) ((OS_TCB_t *) a)->data;
	uint32_t wake_time_b = (uint32_t) ((OS_TCB_t *) b)->data;
	return (int_fast8_t) (wake_time_a - wake_time_b);
}

static void * store[MAX_TASKS];
static heap_t sleep_list = HEAP_INITIALISER(store, MAX_TASKS, wake_time_comparator);

/*
 * Sleep function for specified number of system ticks
 * argument ticks is the number of system ticks to send the task to sleep for
 */
void OS_sleep(uint32_t ticks) {
	/* Get Current Running TCB */
	OS_TCB_t * currentTCB = OS_currentTCB();
	
	/* Store time to wake up task in TCB */
	uint32_t _elapsedTicks = OS_elapsedTicks() + ticks;
	currentTCB->data = &_elapsedTicks;
	/* send task to sleep list and trigger context switch */
	_OS_SVC_WAIT((uint32_t) &sleep_list);
}

/*
 * Verify if any sleeping tasks in the sleep list are ready to be woken
 * IF no tasks are ready to be woken, returns 0
 * ELSE If a task is ready to be woken, removes it from the sleep list
 * and returns a pointer to it
 *
 * NOTE: This is an OS_INTERNAL function and should only ever be called
 * by the context switch to protect the sleep list against race conditions
 */
OS_TCB_t * _OS_verifySleepList(void) {
	/* If sleep list empty */
	if (!sleep_list.size) {
		return 0;
	}
	
	/* get root task in sleep list */
	OS_TCB_t * root_task = heap_peak(&sleep_list);
	
	/* tick times are cast to signed to handle system tick overflow */
	int32_t wake_time = *(int32_t*) (root_task->data);
	int32_t sys_tick = (int32_t) OS_elapsedTicks();
	
	/* If time to wake up, extract sleeping task from heap and return */
	return sys_tick - wake_time > 0 ? heap_extract(&sleep_list) : 0;
}


