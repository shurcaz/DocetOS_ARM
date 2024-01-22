#include "OS/Alloc/static_alloc.h"
#include "stm32f4xx.h"

// 16KB pool size
#define STATIC_ALLOC_POOLSIZE 16384UL
#define STATIC_ALLOC_ALIGNMENT 8U
#define ALIGNMENT_MASK ~(STATIC_ALLOC_ALIGNMENT - 1)

static uint8_t static_pool[STATIC_ALLOC_POOLSIZE] __attribute__ (( aligned(STATIC_ALLOC_ALIGNMENT) ));
static size_t pool_index = STATIC_ALLOC_POOLSIZE;

void * static_alloc(size_t bytes) {
	// If requested bytes is available
	if (bytes <= pool_index) {
		size_t pool_index_tmp;
	
		do {
			// Load pool index
			pool_index_tmp = (size_t) __LDREXW ((volatile uint32_t *) &(pool_index));

			// Reduce pool index by bytes
			pool_index_tmp -= bytes;
		
			// Round index to align data
			pool_index_tmp &= ALIGNMENT_MASK;	
		} while (__STREXW ((uint32_t) pool_index_tmp, (volatile uint32_t *) &(pool_index)));

		return static_pool + pool_index_tmp;
	} else {
		return 0;
	}
}

