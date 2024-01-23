#include "OS/Alloc/mempool.h"
#include "stm32f4xx.h"

/* Allocate a block of memory from the memory pool
 * Argument is a pointer to a memory pool
 * Returns a pointer to allocated memory block
 */
void * pool_allocate(mempool_t * pool) {
	mempool_item_t * block;
	do {
		/* Load head */
		block = (mempool_item_t *) __LDREXW ((volatile uint32_t *) &(pool->head));
		
		/* If pool is empty return 0 */
		if (!block) {
			__CLREX();
			return 0;
		}
		
		/* increment head */
	} while (__STREXW ((uint32_t) block->next, (volatile uint32_t *) &(pool->head)));
	
	return block;
}

/* Deallocate a block of memory back to the memory pool
 * Argument 1 is a pointer to a memory pool
 * Argument 2 is a pointer to a block of memory
 */
void pool_deallocate(mempool_t * pool, void * block) {
	/* Add the new item to the head of the list */
	mempool_item_t * item = block;
	
	do {
		/* Load pool head */
		mempool_item_t * head = (mempool_item_t *) __LDREXW ((volatile uint32_t *) &(pool->head));

		/* Save head to block to add */
		item->next = head;

		/* Store block as head */
	} while (__STREXW ((uint32_t) item, (volatile uint32_t *) &(pool->head)));	
}

/* Initiate the memory pool by allocating it blocks of memory from the static memory pool
 * Argument 1 is a pointer to a memory pool
 * Argument 2 is the requested size of memory blocks
 * Argument 3 is the requested number of memory blocks
 */
void pool_init(mempool_t * pool, size_t blocksize, size_t blocks) {	
	/* Allocate storage for requested blocks */
	uint8_t * storage = static_alloc(blocksize * blocks);
	
	/* Populate memory pool with storage blocks */
	for(uint_fast8_t n = 0; n < blocks; n++) {
		pool_add(pool, storage + (n * blocksize));
	}
}

