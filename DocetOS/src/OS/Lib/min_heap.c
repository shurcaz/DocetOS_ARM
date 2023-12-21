#include "OS/Lib/min_heap.h"


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

/*
 *	Beginning from the end of the heap,
 *	sorts end element up heap
 */
static void heap_up(heap_t *heap) {
	/* 1-Indexed array pointer */
	void * * nodeStore = heap->store - 1;
	
	/* Starting from last element in heap */
	uint_fast8_t n = heap->size;
	
	/* While not root element */
	while (n > 1) {
		/* If node bigger than parent */
		if (heap->comparator(nodeStore[n], nodeStore[n>>1]) < 0) {
			/* Swap child and parent */
			swap(nodeStore + n, nodeStore + (n>>1));
			n = n>>1;
		} else {
			return;
		}
	}
}

/*
 *	Beginning from the root of the heap,
 *	sorts root element down heap
 */
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
		/* NOT and > operator used to avoid <=, equals needed to avoid swapping equal elements*/
		if (!(heap->comparator(nodeStore[n], nodeStore[2*n]) > 0) && 
				!(heap->comparator(nodeStore[n], nodeStore[(2*n)+1]) > 0)) {
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

/*
 *	Insert new item into a heap
 *
 *	argument heap is a pointer to a heap struct
 *	argument item is the item to add to the heap
 *	returns 1 if added successfully, else 0
 */
uint_fast8_t heap_insert(heap_t * heap, void * item) {
	if (heap->size < heap->capacity) {
		/* The new element is always added to the end of a heap */
		heap->store[(heap->size)++] = item;
		
		/* sort new element up heap */
		heap_up(heap);
		
		return 1;
	} else {
		return 0;
	}
}

/*
 *	Extract the root item of a heap
 *
 *	argument heap is a pointer to a heap struct
 *	returns a pointer to the root item, else 0 if heap empty
 */
void * heap_extract(heap_t * heap) {
	/* If heap is not empty */
	if (heap->size) {
		/* extract root */
		void * item = heap->store[0];
		/* move last item in heap to root */
		heap->store[0] = heap->store[--(heap->size)];
		
		/* sort new root down heap */
		heap_down(heap);
		
		/* return extracted root */
		return item;
	}
	
	/* return 0 if heap empty */
	return 0;
}

/*
 *  Peak the root item of the heap
 *
 *  argument heap is a pointer to a heap struct
 *  returns a pointer to the root item
 */
void * heap_peak(heap_t * heap) {
	return heap->store[0];
}

