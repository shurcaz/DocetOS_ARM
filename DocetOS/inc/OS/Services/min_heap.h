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
	int_fast8_t (*comparator) (void *, void *);
	
	uint32_t size;
} heap_t;

#define HEAP_INITIALISER(store, comparator) { .store = (store), .size = 0, .comparator = (comparator) }


void heap_insert(heap_t *heap, void *value);

void * heap_extract(heap_t *heap);

uint_fast8_t heap_isEmpty(heap_t *heap);

#endif /* _MIN_HEAP_H_ */
