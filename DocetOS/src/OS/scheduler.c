#define OS_INTERNAL

#include "OS/scheduler.h"
#include "OS/os.h"
#include "OS/Lib/min_heap.h"

#include "stm32f4xx.h"

#include <string.h>

/* TODO - replace below comment */
/* This is an implementation of an extremely simple round-robin scheduler.

   A task list structure is declared.  Tasks are added to the list to create a circular buffer.
	 When tasks finish, they are removed from the list.  When the scheduler is invoked, it simply
	 advances the head pointer, and returns it.  If the head pointer is null, a pointer to the
	 idle task is returned instead.
	 
	 The scheduler is reasonably efficient but not very flexible.  The "yield" flag is not
	 checked, but merely cleared before a task is returned, so OS_yield() is equivalent to
	 OS_schedule() in this implementation.
*/

/* comparison function and initialisation of priority heap */
static inline int_fast8_t priority_comparator(void * a, void * b) {
	uint16_t priority_a = (*((OS_TCB_t * *) a))->current_priority;
	uint16_t priority_b = (*((OS_TCB_t * *) b))->current_priority;
	return (int_fast8_t) (priority_a - priority_b);
}

static void * heap_store[PRIORITY_LEVELS];
static heap_t priority_heap = HEAP_INITIALISER(heap_store, PRIORITY_LEVELS, priority_comparator);

static _OS_FPSheduler_t _OS_FPSheduler = { .scheduler = &priority_heap, 
																					 .priority_list = {0}, 
																					 .pending_list_head = 0 };

/* 
 * Add task to priority level round robin 
 * argument queue_head_ptr is a pointer to the head pointer in the priority list array
 * argument task is the task to be added to the priority level round robin
 */
static void list_add(OS_TCB_t * * queue_head_ptr, OS_TCB_t * task) {	
	/* If priority level is not empty */
	if (*queue_head_ptr) {
		/* set tasks adjacent tasks */
		task->next = *queue_head_ptr;
		task->prev = (*queue_head_ptr)->prev;
		
		/* update adjacent tasks */
		task->prev->next = task;
		(*queue_head_ptr)->prev = task;
	} else {
		/* set task adjacent tasks to self */
		task->next = task;
		task->prev = task;
		
		/* update head */
		*queue_head_ptr = task;
		
		/* add priority level to scheduler heap */
		heap_insert(_OS_FPSheduler.scheduler, queue_head_ptr);
	}
}

/* 
 * Remove task from priority level round robin 
 * argument queue_head_ptr is a pointer to the head pointer in the priority list array
 * argument task is the task to be removed from the priority level round robin
 */
static void list_remove(OS_TCB_t * * queue_head_ptr, OS_TCB_t * task) {
	/* If task to be removed is the list head */
	if (*queue_head_ptr == task) {
		/* If task is last item in list */
		if (task->next == task) {
			*queue_head_ptr = 0;
			return;
		}
		
		/* Increment head */
		*queue_head_ptr = task->next;
	}
	
	/* Update adjacent tasks */
	task->prev->next = task->next;
	task->next->prev = task->prev;
}

/* Round-robin scheduler */
OS_TCB_t const * _OS_schedule(void) {	
	/* remove empty priority levels from scheduler heap */
	while (!*((OS_TCB_t * *) heap_peak(_OS_FPSheduler.scheduler))) {
		heap_extract(_OS_FPSheduler.scheduler);
	}
	
	/* If priority heap not empty	*/
	if (_OS_FPSheduler.scheduler->size) {
		/* get root item */
		OS_TCB_t * * ptr_to_head_ptr = (OS_TCB_t * *) heap_peak(_OS_FPSheduler.scheduler);
		OS_TCB_t * item = *ptr_to_head_ptr;
		
		/* reset yield flag */
		item->state &= ~TASK_STATE_YIELD;
		
		/* increment head */
		*ptr_to_head_ptr = item->next;
		
		/* return root item */
		return item;
	}
	
	/* No tasks are runnable, so return the idle task */
	return _OS_idleTCB_p;
}

/* Initialises a task control block (TCB) and its associated stack.  See os.h for details. */
void OS_initialiseTCB(OS_TCB_t * TCB, uint32_t * const stack, void (* const func)(void const * const), void const * const data, uint16_t priority) {
	TCB->sp = stack - (sizeof(_OS_StackFrame_t) / sizeof(uint32_t));
	TCB->state = 0;
	TCB->prev = TCB->next = 0;
	TCB->current_priority = TCB->initial_priority = priority;
	TCB->pending_heap = 0;
	_OS_StackFrame_t *sf = (_OS_StackFrame_t *)(TCB->sp);
	/* By placing the address of the task function in pc, and the address of _OS_task_end() in lr, the task
	   function will be executed on the first context switch, and if it ever exits, _OS_task_end() will be
	   called automatically */
	*sf = (_OS_StackFrame_t) {
		.r0 = (uint32_t)(data),
		.r1 = 0,
		.r2 = 0,
		.r3 = 0,
		.r4 = 0,
		.r5 = 0,
		.r6 = 0,
		.r7 = 0,
		.r8 = 0,
		.r9 = 0,
		.r10 = 0,
		.r11 = 0,
		.r12 = 0,
		.lr = (uint32_t)_OS_task_end,
		.pc = (uint32_t)(func),
		.psr = xPSR_T_Msk  /* Sets the thumb bit to avoid a big steaming fault */
	};
}

/* 'Add task' */
void OS_addTask(OS_TCB_t * const tcb) {
	OS_TCB_t * * priority_level_head = &(_OS_FPSheduler.priority_list[tcb->current_priority]);
	list_add(priority_level_head, tcb);
}

/* SVC handler that's called by _OS_task_end when a task finishes.  Removes the
   task from the scheduler and then queues PendSV to reschedule. */
void _OS_taskExit_delegate(void) {
	/* Remove the given TCB from the list of tasks so it won't be run again */
	OS_TCB_t * tcb = OS_currentTCB();
	OS_TCB_t * * priority_level_head = &(_OS_FPSheduler.priority_list[tcb->current_priority]);
	list_remove(priority_level_head, tcb);
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}
