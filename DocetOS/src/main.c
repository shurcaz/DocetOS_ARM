#include "OS/os.h"
#include "Utils/utils.h"
#include <stdio.h>

static void task1(void const *const args) {
	for (uint_fast16_t i = 0; i < 1000; ++i) {
		printf("AAAAAAAA");
	}
}

static void task2(void const *const args) {
	while (1) {
		printf("BBBBBBBB");
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
	
	/* Add the tasks to the scheduler */

	/* Start the OS */
	
}
