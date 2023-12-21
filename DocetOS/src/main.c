#include "OS/os.h"
#include "Utils/utils.h"
#include "OS/sleep.h"
#include <stdio.h>

static void task1(void const *const args) {
	(void) args; // Suppress unused variable warning
	for (uint_fast16_t i = 0; i < 1000; ++i) {
		printf("AAAAAAAA");
		OS_sleep(1000);
	}
}

__attribute__ (( noreturn ))
static void task2(void const *const args) {
	(void) args; // Suppress unused variable warning
	while (1) {
		printf("\nBBBBBBBB");
		OS_sleep(300);
	}
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
	static OS_TCB_t TCB1, TCB2;

	/* Initialise the TCBs using the two functions above */
	OS_initialiseTCB(&TCB1, stack1 + 128, task1, NULL, 1);
	OS_initialiseTCB(&TCB2, stack2 + 128, task2, NULL, 2);
	
	/* Add the tasks to the scheduler */
	OS_addTask(&TCB1);
	OS_addTask(&TCB2);

	/* Start the OS */
	OS_start();
}
