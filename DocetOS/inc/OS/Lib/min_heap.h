#ifndef _MIN_HEAP_H_
#define _MIN_HEAP_H_

#include <stdint.h>

/*========================*/
/*      EXTERNAL API      */
/*========================*/

/* Generic min-heap data structure */
typedef struct {
	/* Pointer to a pointer to a memory location storing data */
	void * * const store;
	
	/* Pointer to a function used to compare data in the heap */
	int_fast16_t (* comparator) (void *, void *);
	
	uint32_t size;
	uint32_t capacity;
} heap_t;

#define HEAP_INITIALISER(store_array, cap, comparator_function) { .store = (store_array), .size = 0, .capacity = (cap), .comparator = (comparator_function) }


/*
 *	Insert new item into a heap
 *
 *	argument heap is a pointer to a heap struct
 *	argument item is the item to add to the heap
 *	returns 1 if added successfully, else 0
 */
uint_fast8_t heap_insert(heap_t * heap, void * value);

/*
 *	Extract the root item of a heap
 *
 *	argument heap is a pointer to a heap struct
 *	returns a pointer to the root item, else 0 if heap empty
 */
void * heap_extract(heap_t * heap);

/*
 *  Peak the root item of the heap
 *
 *  argument heap is a pointer to a heap struct
 *  returns a pointer to the root item
 */
void * heap_peak(heap_t * heap);

#endif /* _MIN_HEAP_H_ */
