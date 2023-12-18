#include "OS/Services/min_heap.h"


/* ##########################  Internal Heap Functions  ########################## */

/*
 *	Swap the location of 2 pointers in the heap
 *
 * 	argument addr1 is pointer 1 in the heap
 * 	argument addr2 is pointer 2 in the heap
 */
static void swap(void * * addr1, void * * addr2) {
	void * temp = *addr1;
	*addr1 = *addr2;
	*addr2 = temp;
}

static void heap_up(heap_t *heap) {
	/* 1-Indexed array pointer */
	void * * nodeStore = heap->store - 1;
	
	/* Starting from last element in heap */
	uint_fast8_t n = heap->size;
	
	/* While not root element */
	while (n > 1) {
		/* If node bigger than parent */
		if (heap->comparator(nodeStore[n], nodeStore[n>>1]) > 0) {
			return;
		} else {
			/* Swap child and parent */
			swap(nodeStore + n, nodeStore + (n>>1));
			n = n>>1;
		}
	}
}

static void heap_down(heap_t *heap) {
	/* 1-Indexed array pointer */
	void * * nodeStore = heap->store - 1;
	
	/* Starting with root element */
	uint_fast8_t n = 1;
	
	/* if element has no children, stop */
	while (2*n <= heap->size) {
		
		/* If Second child does not exist */
		if ((2*n)+1 > heap->size) {
			/* Swap parent and first child if child smaller */
			if (heap->comparator(nodeStore[n], nodeStore[2*n]) > 0) {
				swap(nodeStore + n, nodeStore + (2*n));
			}
		}
		
		/* if the children are bigger than parent, STOP */
		if (heap->comparator(nodeStore[n], nodeStore[2*n]) < 0 && heap->comparator(nodeStore[n], nodeStore[(2*n)+1]) < 0) {
			return;
		}
		
		/* Find smallest child */
		/* If first child smaller than second */
		if (heap->comparator(nodeStore[2*n], nodeStore[(2*n)+1]) < 0) {
			swap(nodeStore + n, nodeStore + (2*n));
			n *= 2;
		} else {
			swap(nodeStore + n, nodeStore + (2*n) + 1);
			n = (2*n) + 1;
		}
	}
}

/* ##############################  External Functions  ########################### */

void heap_insert(heap_t *heap, void * item) {
	/* The new element is always added to the end of a heap */
	heap->store[(heap->size)++] = item;
	heap_up(heap);
}

void * heap_extract(heap_t * heap) {
	/* The root value is extracted, and the space filled by the value from the end */
	void * item = heap->store[0];
	if (heap_isEmpty(heap)) {
		heap->store[0] = heap->store[--(heap->size)];
		heap_down(heap);
	}
	return item;
}

uint_fast8_t heap_isEmpty(heap_t *heap) {
	return !(heap->size);
}

