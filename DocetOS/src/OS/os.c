#define OS_INTERNAL

#include "OS/os.h"
#include "stm32f4xx.h"

#include <stdlib.h>

__ALIGNED(8)

/* Idle task stack frame area and TCB.  The TCB is not declared const, to ensure that it is placed in writable
   memory by the compiler.  The pointer to the TCB _is_ declared const, as it is visible externally - but it will
   still be writable by the assembly-language context switch. */
static _OS_StackFrame_t _idleTaskSF;

static OS_TCB_t _OS_idleTCB = {
	.sp = (void *)(&_idleTaskSF + 1),
	.state = 0
};

OS_TCB_t const * const _OS_idleTCB_p = &_OS_idleTCB;

/* Total elapsed ticks */
static volatile uint32_t _ticks = 0;

/* GLOBAL: Holds pointer to current TCB.  DO NOT MODIFY, EVER. */
OS_TCB_t * volatile _currentTCB = 0;
/* Getter for the current TCB pointer.  Safer to use because it can't be used
   to change the pointer itself. */
OS_TCB_t * OS_currentTCB(void) {
	return _currentTCB;
}

/* Getter for the current time. */
uint32_t OS_elapsedTicks(void) {
	return _ticks;
}

/* IRQ handler for the system tick.  Schedules PendSV */
// Local prototype - overrides weak export but is not part of the API
void SysTick_Handler(void) {
	_ticks = _ticks + 1;
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* SVC handler for OS_yield().  Sets the TASK_STATE_YIELD flag and schedules PendSV */
void _OS_yield_delegate(void) {
	_currentTCB->state |= TASK_STATE_YIELD;
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* SVC handler for OS_schedule().  Simply schedules PendSV */
void _OS_schedule_delegate(void) {
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

/* Starts the OS and never returns. */
void OS_start(void) {
	// This call never returns (and enables interrupts and resets the stack)
	_task_init_switch(&_OS_idleTCB);
}

/* Function that's called by a task when it ends (the address of this function is
   inserted into the link register of the initial stack frame for a task).  Invokes a SVC
   call (see os.h); the handler is _OS_taskExit_delegate() (see scheduler.c). */
void _OS_task_end(void) {
	_OS_task_exit();
	/* DO NOT STEP OUT OF THIS FUNCTION when debugging!  PendSV must be allowed to run
	   and switch tasks.  A hard fault awaits if you ignore this.
		 If you want to see what happens next, or debug something, set a breakpoint at the
	   start of PendSV_Handler (see os_asm.s) and hit 'run'. */
}

/* SVC handler to enable systick from unprivileged code */
void _OS_enable_systick_delegate(void) {
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / 1000);
	NVIC_SetPriority(SysTick_IRQn, 0x10);
}
