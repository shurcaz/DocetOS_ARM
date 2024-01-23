#include "OS/os.h"
#include "Utils/utils.h"
#include "OS/sleep.h"
#include "OS/mutex.h"
#include "OS/Alloc/mempool.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define USE_PRIORITY_INHERITANCE 1

/* packet storage mutex */
static void * store[MAX_TASKS] __attribute__ (( aligned(8) ));
static heap_t wait_list = HEAP_INITIALISER(store, MAX_TASKS, OS_mutex_comparator);
static OS_mutex_t data_store_mutex = OS_MUTEX_STATIC_INITIALISER(&wait_list, USE_PRIORITY_INHERITANCE);

/* Example data packet structure, just for demonstration purposes */
typedef struct {
	uint32_t id;
	char data[12];
} packet_t;

/* Memory pool */
static mempool_t pool = MEMPOOL_INITIALISER;

static packet_t * packet_buffer[10];
static uint16_t packet_count = 0;

/* Populate pack data field associated string */
static void populate_packet_data(packet_t * p) {
	char * strings[10] = {"first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eigth", "ninth", "tenth"};
	strncpy(p->data, strings[p->id], 10);
}

/* Empty data buffer and print values,
 * returns packet memory to the memory pool */
static void empty_buffer(void) {
	printf("SENDING DATA TO EARTH\r\n");
	for (uint_fast8_t i = 0; i < 10; ++i) {
		printf("Sending packet (id %" PRIu32 ", data '%s') at address %p\r\n", packet_buffer[i]->id, packet_buffer[i]->data, (void *) packet_buffer[i]);
		pool_deallocate(&pool, (void *) packet_buffer[i]);
	}
}

/* LOW PRIORITY TASK */
/* Creates data packets */
__attribute__ (( noreturn ))
static void task1(void const *const args) {
	(void) args; // Suppress unused variable warning
	while (1) {
		// Fill 10 values
		OS_mutex_acquire(&data_store_mutex);
		printf("\nBEGIN DATA COLLECTION\r\n");
		for (uint_fast8_t j = 0; j < 10; ++j) {
			// fill value
			printf("Collecting Science Data Packet ID: %d\r\n", j);
			
			// Create packet and add to array
			packet_buffer[j] = pool_allocate(&pool);
			packet_buffer[j]->id = j;
			populate_packet_data(packet_buffer[j]);
			packet_count++;
		
			for (uint_fast32_t i = 0; i < 100; ++i) {}
		}
		OS_mutex_release(&data_store_mutex);
		OS_sleep(10);
	}
}

/* MEDIUM PRIORITY TASK */
/* Simulation of misc background task */
__attribute__ (( noreturn ))
static void task2(void const * const args) {
	(void) args; // Suppress unused variable warning
	while(1) {
		OS_sleep(5);
		
		// Simulate execution time
		for (uint_fast32_t i = 0; i < 10000000; ++i) {}
	}
}

/* HIGH PRIORITY TASK */
/* 
 * Polls data buffer regularly to check if full,
 * if full, empties buffer and prints values
 */
__attribute__ (( noreturn ))
static void task3(void const *const args) {
	(void) args; // Suppress unused variable warning
	while(1) {
		OS_sleep(100);
		
		OS_mutex_acquire(&data_store_mutex);
		if (packet_count == 10) {
			printf("\nBUFFER FULL:\r\n");
			// process packets
			packet_count = 0;
			empty_buffer();
		}
		OS_mutex_release(&data_store_mutex);
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
	static uint32_t stack3[128] __attribute__ (( aligned(8) ));
	static OS_TCB_t TCB1, TCB2, TCB3;

	/* Initialise the TCBs using the two functions above */
	OS_initialiseTCB(&TCB1, stack1 + 128, task1, NULL, 12);
	OS_initialiseTCB(&TCB2, stack2 + 128, task2, NULL, 8);
	OS_initialiseTCB(&TCB3, stack3 + 128, task3, NULL, 4);
	
	/* Add the tasks to the scheduler */
	OS_addTask(&TCB1);
	OS_addTask(&TCB2);
	OS_addTask(&TCB3);
	
	/* Fill memory pool */
	pool_init(&pool, sizeof(packet_t), 10);
	
	/* Start the OS */
	OS_start();
}





