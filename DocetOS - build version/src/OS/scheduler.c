#define OS_INTERNAL

#include "OS/scheduler.h"
#include "OS/os.h"
#include "OS/sleep.h"
#include "OS/Lib/min_heap.h"

#include "stm32f4xx.h"
#include <string.h>

/* This is an implementation of a fixed-priority round-robin scheduler.
	 An array of task lists are declared and a binary min-heap to track occupied priority levels.
	 Task lists act as circular buffers, tasks are added to the list associated with their priority.
	 When tasks finish, they are removed from the list. When the scheduler is invoked, it simply
	 advances the head pointer of the highest priority task list, and returns it.  If there are no tasks
   in the scheduler, a pointer to the idle task is returned instead.
*/

/* ##########################  Scheduler Initialisation  ########################## */

/*
 *	priority heap comparitor function
 *	Compares the priority of 2 control blocks
 * 	Takes pointers a and b and casts them to pointers to pointers to task control blocks.
 * 	Returns:
 *			positive if priority a is larger than b
 *			zero if priority a and b are equal
 *			negative if priority a is smaller than b
 */
static inline int_fast8_t priority_comparator(void * a, void * b) {
	uint16_t priority_a = (*((OS_TCB_t * *) a))->current_priority;
	uint16_t priority_b = (*((OS_TCB_t * *) b))->current_priority;
	return (int_fast8_t) (priority_a - priority_b);
}

/* Initialisate scheduler */
static void * heap_store[PRIORITY_LEVELS];
static heap_t priority_heap = HEAP_INITIALISER(heap_store, PRIORITY_LEVELS, priority_comparator);

static _OS_FPSheduler_t _OS_FPSheduler = { .scheduler = &priority_heap, .priority_list = {0} };


/* #############################  Internal Functions  ############################# */

																 
/* 
 * Add task to priority level round robin 
 * argument queue_head_ptr is a pointer to the head pointer in the priority list array
 * argument task is the task to be added to the priority level round robin
 */
static void scheduler_add(OS_TCB_t * tcb) {	
	OS_TCB_t * * queue_head_ptr = &(_OS_FPSheduler.priority_list[tcb->current_priority]);
	
	/* If priority level is not empty */
	if (*queue_head_ptr) {
		/* set tasks adjacent tasks */
		tcb->next = *queue_head_ptr;
		tcb->prev = (*queue_head_ptr)->prev;
		
		/* update adjacent tasks */
		tcb->prev->next = tcb;
		(*queue_head_ptr)->prev = tcb;
	} else {
		/* set task adjacent tasks to self */
		tcb->next = tcb;
		tcb->prev = tcb;
		
		/* update head */
		*queue_head_ptr = tcb;
		
		/* add priority level to scheduler heap */
		heap_insert(_OS_FPSheduler.scheduler, queue_head_ptr);
	}
}

/* 
 * Remove task from priority level round robin 
 * argument queue_head_ptr is a pointer to the head pointer in the priority list array
 * argument task is the task to be removed from the priority level round robin
 */
static void scheduler_remove(OS_TCB_t * tcb) {
	OS_TCB_t * * queue_head_ptr = &(_OS_FPSheduler.priority_list[tcb->current_priority]);
	
	/* If task to be removed is the list head */
	if (*queue_head_ptr == tcb) {
		/* If task is last item in list */
		if (tcb->next == tcb) {
			*queue_head_ptr = 0;
			return;
		}
		
		/* Increment head */
		*queue_head_ptr = tcb->next;
	}
	
	/* Update adjacent tasks */
	tcb->prev->next = tcb->next;
	tcb->next->prev = tcb->prev;
}

/* Round-robin scheduler 
 *  - Verifies and wakes sleeping tasks
 *  - Empties scheduler heap of empty priority levels
 *  - Advances head pointer of highest priority level and returns it
 *  - If scheduler heap empty, returns idle task pointer
 */
OS_TCB_t const * _OS_schedule(void) {
	/* Verify and wake any tasks that can be woken */
	OS_TCB_t * sleepy_task;
	while (( sleepy_task = _OS_verifySleepList() )) {
		/* Add task to scheduler */
		scheduler_add(sleepy_task);	
	}
	
	/* remove empty priority levels from scheduler heap */
	while (_OS_FPSheduler.scheduler->size && !*((OS_TCB_t * *) heap_peek(_OS_FPSheduler.scheduler))) {
		/* If scheduler heap is not empty and the root priority level is zero
			 Remove top priority level from scheduler heap */
		heap_extract(_OS_FPSheduler.scheduler);
	}
	
	/* If priority heap not empty	*/
	if (_OS_FPSheduler.scheduler->size) {
		/* get root item */
		OS_TCB_t * * ptr_to_head_ptr = (OS_TCB_t * *) heap_peek(_OS_FPSheduler.scheduler);
		
		/* increment head */
		*ptr_to_head_ptr = (*ptr_to_head_ptr)->next;
		
		/* reset yield flag */
		(*ptr_to_head_ptr)->state &= ~TASK_STATE_YIELD;
		
		/* return root item */
		return *ptr_to_head_ptr;
	}
	
	/* No tasks are runnable, so return the idle task */
	return _OS_idleTCB_p;
}


/* #############################  External Functions  ############################# */


/* Initialises a task control block (TCB) and its associated stack.  See os.h for details. */
void OS_initialiseTCB(OS_TCB_t * TCB, uint32_t * const stack, void (* const func)(void const * const), void const * const data, uint16_t priority) {
	TCB->sp = stack - (sizeof(_OS_StackFrame_t) / sizeof(uint32_t));
	TCB->state = 0;
	TCB->prev = TCB->next = 0;
	TCB->current_priority = TCB->initial_priority = priority;
	TCB->data = 0;
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

/* Adds a task control block to the scheduler
 * argument is a pointer to a task control block
 */
void OS_addTask(OS_TCB_t * const tcb) {
	scheduler_add(tcb);
}


/* ################################  SVC Handlers  ################################ */


/* SVC handler that's called by _OS_task_end when a task finishes.  Removes the
   task from the scheduler and then queues PendSV to reschedule. */
void _OS_taskExit_delegate(void) {
	/* Remove the given TCB from the list of tasks so it won't be run again */
	OS_TCB_t * tcb = OS_currentTCB();
	scheduler_remove(tcb);
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* OS_WAIT SVC delegate
 * Removes the current task from the scheduler and adds it to a waiting list
 * argument is a stack containing a pointer to a wait list heap
 */
void _OS_wait_delegate(void * const stack) {
	/* Get wait_list heap */
	_OS_SVC_StackFrame_t * const s = (_OS_SVC_StackFrame_t * const) stack;
	heap_t * wait_list = (heap_t *) s->r0;
	
	/* Get running task */
	OS_TCB_t * currentTCB = OS_currentTCB();
	
	/* Remove task from scheduler */
	scheduler_remove(currentTCB);
	
	/* Add to wait_list */
	heap_insert(wait_list, currentTCB);
	
	/* Trigger context switch */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* OS_NOTIFY SVC delegate
 * Removes the current task from a waiting list and adds it to the scheduler
 * argument is a stack containing a pointer to a wait list heap
 */
void _OS_notify_delegate(void * const stack){
	/* Get wait_list heap */
	_OS_SVC_StackFrame_t * const s = (_OS_SVC_StackFrame_t * const) stack;
	heap_t * wait_list = (heap_t *) s->r0;
	
	/* Get notified task */
	OS_TCB_t * tcb = heap_extract(wait_list);
	
	if (tcb) {
		/* Add task to scheduler */
		scheduler_add(tcb);
	}
	
	/* Trigger context switch */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* OS_MODIFY_PRIORITY SVC delegate
 * Removes the current task the scheduler, modifies it's priority, and returns it
 * argument is a stack containing a pointer to a task and a new priority value
 */
void _OS_modifyPriority_delegate(void * const stack){
	/* Get new task priority */
	_OS_SVC_StackFrame_t * const s = (_OS_SVC_StackFrame_t * const) stack;
	OS_TCB_t * tcb = (OS_TCB_t *) s->r0;
	uint16_t new_priority = (uint16_t) s->r1;
	
	if (new_priority != tcb->current_priority) {
		/* Remove task from scheduler */
		scheduler_remove(tcb);
		
		/* Update Priority */
		tcb->current_priority = new_priority;
		
		/* Add task back to scheduler */
		scheduler_add(tcb);
	}
	
}

