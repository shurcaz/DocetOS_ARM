#ifndef __scheduler_h__
#define __scheduler_h__

#include <stdint.h>
#include "OS/Lib/min_heap.h"

/***************************************/
/* Scheduler Configuration Definitions */
/***************************************/

#define PRIORITY_LEVELS 16			// Number of priority levels
#define MAX_TASKS 			64			// Maximum number of tasks 

/*========================*/
/*      EXTERNAL API      */
/*========================*/

/* Task Control Block Structure*/
typedef struct s_OS_TCB_t {
	/* Task stack pointer.  It's important that this is the first entry in the structure,
	   so that a simple double-dereference of a TCB pointer yields a stack pointer. */
	void * volatile sp;
	
	/* Field used to describe task state, current assigned flags:
	 * bit 0 - yield flag, task has yielded and triggered a context switch
	 */
	uint32_t volatile state;
	
	/* pointers to adjacent TCBs' in round-robin */
	struct s_OS_TCB_t * prev;
	struct s_OS_TCB_t * next;
	
	/* Task priority trackers */
	uint16_t volatile current_priority;
	uint16_t initial_priority;
	
	/* Misc data pointer to be used for heap specific operations */
	void * data;
} OS_TCB_t;

/******************************************/
/* Task creation and management functions */
/******************************************/

/* Initialises a task control block (TCB) and its associated stack.  The stack is prepared
   with a frame such that when this TCB is first used in a context switch, the given function will be
   executed.  If and when the function exits, a SVC call will be issued to kill the task, and a callback
   will be executed.
   The first argument is a pointer to a TCB structure to initialise.
   The second argument is a pointer to the TOP OF a region of memory to be used as a stack (stacks are full descending).
     Note that the stack MUST be 8-byte aligned.  This means if (for example) malloc() is used to create a stack,
     the result must be checked for alignment, and then the stack size must be added to the pointer for passing
     to this function.
   The third argument is a pointer to the function that the task should execute.
   The fourth argument is a void pointer to data that the task should receive. 
	 The fifth argument is the initial priority of the task.
*/
void OS_initialiseTCB(OS_TCB_t * TCB, uint32_t * const stack, void (* const func)(void const * const), void const * const data, uint16_t priority);

/* Adds a task control block to the scheduler
 * argument is a pointer to a task control block
 */
void OS_addTask(OS_TCB_t * const tcb);

/*========================*/
/*      INTERNAL API      */
/*========================*/

#ifdef OS_INTERNAL

/* Context Switch */
OS_TCB_t const * _OS_schedule(void);

/* Fixed Priority Scheduler Structure */
typedef struct s_OS_FPSheduler_t {
	/* Scheduler heap, holds pointers to heads of priority level round-robins */
	heap_t * scheduler;
	
	/* Array of pointers to each priority level list head */
	OS_TCB_t * priority_list[PRIORITY_LEVELS];
} _OS_FPSheduler_t;

/* SVC delegates */
void _OS_taskExit_delegate(void);
void _OS_wait_delegate(void * const stack);
void _OS_notify_delegate(void * const stack);
void _OS_modifyPriority_delegate(void * const stack);

/* Constants that define bits in a thread's 'state' field. */
#define TASK_STATE_YIELD    					(1UL << 0) // Bit zero is the 'yield' flag

#endif /* os_internal */

#endif /* __scheduler_h__ */
