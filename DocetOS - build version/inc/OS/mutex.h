#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "OS/scheduler.h"

typedef struct s_OS_mutex_t {
	OS_TCB_t * volatile owner_tcb;
	volatile uint32_t counter;
	heap_t * wait_list;
	uint32_t check_code;
	uint32_t use_priority_inheritance;
} OS_mutex_t;

#define OS_MUTEX_STATIC_INITIALISER(heap, inheritance_bool) { .owner_tcb = 0, .counter = 0, .check_code = 0, .wait_list = (heap), .use_priority_inheritance = (inheritance_bool) }

/* comparison function and initialisation of mutex waiting list heap */
int_fast8_t OS_mutex_comparator(void * a, void * b);

/* Attempts to acquire a mutex for the currently running task.
 * If the mutex is already owned, sends the currently running
 * task to the mutex waiting list. If the mutex is owned by a
 * task with a lower priority, the mutex owner inherits the higher
 * priority.
 * argument is a pointer to a mutex
 */
void OS_mutex_acquire(OS_mutex_t * mutex);

/* Attempts to release a mutex for the currently running task.
 * If the running task had its priority ascended, returns the
 * task priority to its initial value
 * argument is a pointer to a mutex
 */
void OS_mutex_release(OS_mutex_t * mutex);

#endif /* MUTEX_H */
