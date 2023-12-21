#include "OS/os.h"
#include "Utils/utils.h"
#include "OS/sleep.h"
#include "OS/mutex.h"
#include <stdio.h>

static void * store[MAX_TASKS];
static heap_t wait_list = HEAP_INITIALISER(store, MAX_TASKS, OS_mutex_comparator);
static OS_mutex_t printf_mutex = OS_MUTEX_STATIC_INITIALISER(&wait_list);

__attribute__ (( noreturn ))
static void task1(void const *const args) {
	(void) args; // Suppress unused variable warning
	while (1) {
		OS_mutex_acquire(&printf_mutex);
		for (uint_fast16_t i = 0; i < 5000; ++i) {
			printf("\nLOW");
		}
		OS_mutex_release(&printf_mutex);
	}
}

__attribute__ (( noreturn ))
static void task2(void const *const args) {
	(void) args; // Suppress unused variable warning
	OS_sleep(500);
	while (1) {
		printf("\nMEDIUM");
	}
}

//__attribute__ (( noreturn ))
static void task3(void const *const args) {
	(void) args; // Suppress unused variable warning
	OS_sleep(5000);
	OS_mutex_acquire(&printf_mutex);
	for (uint_fast16_t i = 0; i < 200; ++i) {
		printf("\nHIGH");
	}
	OS_mutex_release(&printf_mutex);
}

/* MAIN FUNCTION */

int main(void) {
	
	configClock();
	configUSART2(38400);
	
	printf("\r\nDocetOS\r\n");
	
	/* Reserve memory for two stacks and two TCBs.
	   Remember that stacks must be 8-byte aligned. */
	static uint32_t stack1[128] __attribute__ (( aligned(8) ));
	static uint32_t stack2[128] __attribute__ (( aligned(8) ));
	static uint32_t stack3[128] __attribute__ (( aligned(8) ));
	static OS_TCB_t TCB1, TCB2, TCB3;

	/* Initialise the TCBs using the two functions above */
	OS_initialiseTCB(&TCB1, stack1 + 128, task1, NULL, 12);
	OS_initialiseTCB(&TCB2, stack2 + 128, task2, NULL, 8);
	OS_initialiseTCB(&TCB3, stack3 + 128, task3, NULL, 5);
	
	/* Add the tasks to the scheduler */
	OS_addTask(&TCB1);
	OS_addTask(&TCB2);
	OS_addTask(&TCB3);

	/* Start the OS */
	OS_start();
}
