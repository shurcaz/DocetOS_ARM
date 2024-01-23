#define OS_INTERNAL
#include "OS/mutex.h"
#include "OS/os.h"
#include "stm32f4xx.h"

/* comparison function and initialisation of mutex waiting list heap */
inline int_fast8_t OS_mutex_comparator(void * a, void * b) {
	uint16_t priority_a = (*((OS_TCB_t * *) a))->current_priority;
	uint16_t priority_b = (*((OS_TCB_t * *) b))->current_priority;
	return (int_fast8_t) (priority_a - priority_b);
}

/* Attempts to acquire a mutex for the currently running task.
 * If the mutex is already owned, sends the currently running
 * task to the mutex waiting list. If the mutex is owned by a
 * task with a lower priority, the mutex owner inherits the higher
 * priority.
 * argument is a pointer to a mutex
 */
void OS_mutex_acquire(OS_mutex_t * mutex) {	
	OS_TCB_t * mutex_tcb, * current_tcb;
	uint32_t cc;
	while (1) {
		cc = mutex->check_code;
		current_tcb = OS_currentTCB();
		mutex_tcb = (OS_TCB_t *) __LDREXW ((volatile uint32_t *) &(mutex->owner_tcb));
		
		if (!mutex_tcb) {
			// If mutex unowned
			if (__STREXW ((uint32_t) current_tcb, (volatile uint32_t *) &(mutex->owner_tcb))) {
				// If store fails
				continue;
			} else {
				// If store succeeds
				mutex->counter++;
				break;	
			}	
		} else {
			// If mutex owned
			if (mutex_tcb == current_tcb) {
				// If mutex owned by current tcb
				mutex->counter++;
				__CLREX();
				break;
			} else {
				// If mutex owned by another tcb
				if (cc == mutex->check_code) {
					// Inherit higher priority if priority inheritance active					
					if ((current_tcb->current_priority < mutex->owner_tcb->current_priority) && mutex->use_priority_inheritance) {
						_OS_SVC_MODIFY_PRIORITY((uint32_t) mutex->owner_tcb, (uint16_t) current_tcb->current_priority);
					}					
					_OS_SVC_WAIT((uint32_t) mutex->wait_list);
				}
				continue;
			}
		}
	}
}
	
/* Attempts to release a mutex for the currently running task.
 * If the running task had its priority ascended, returns the
 * task priority to its initial value
 * argument is a pointer to a mutex
 */
void OS_mutex_release(OS_mutex_t * mutex) {
	// If mutex owned by tcb calling release
	OS_TCB_t * current_tcb = OS_currentTCB();
	if (current_tcb == mutex->owner_tcb) {
		mutex->counter--;
		if (!mutex->counter) {
			mutex->owner_tcb = 0;
			mutex->check_code++;
			
			// If priority modified by inheritance, reset priority
			if (current_tcb->current_priority != current_tcb->initial_priority) {
				_OS_SVC_MODIFY_PRIORITY((uint32_t) current_tcb, (uint16_t) current_tcb->initial_priority);
			}
			
			_OS_SVC_NOTIFY((uint32_t) mutex->wait_list);
		}
	}
}
