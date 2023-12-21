#ifndef SLEEP_H
#define SLEEP_H

#include <stdint.h>
#include "OS/scheduler.h"

/*
 * Sleep function for specified number of system ticks
 * argument ticks is the number of system ticks to send the task to sleep for
 */
void OS_sleep(uint32_t ticks);

#ifdef OS_INTERNAL

/*
 * Verify if any sleeping tasks in the sleep list are ready to be woken
 * IF no tasks are ready to be woken, returns 0
 * ELSE If a task is ready to be woken, removes it from the sleep list
 * and returns a pointer to it
 *
 * NOTE: This is an OS_INTERNAL function and should only ever be called
 * by the context switch to protect the sleep list against race conditions
 */
OS_TCB_t * _OS_verifySleepList(void);

#endif /* os_internal */

#endif /* SLEEP_H */
