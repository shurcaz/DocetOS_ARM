#ifndef MEMPOOL_H
#define MEMPOOL_H
#include <stddef.h>
#include "OS/Alloc/static_alloc.h"

// Memory Pool Struct
struct mempool_item_s {
	struct mempool_item_s *next;
};
typedef struct mempool_item_s mempool_item_t;

// Memory Block Struct
typedef struct {
	mempool_item_t *head;
} mempool_t;

#define MEMPOOL_INITIALISER { .head = 0 }

/* Allocate a block of memory from the memory pool
 * Argument is a pointer to a memory pool
 * Returns a pointer to allocated memory block
 */
void *pool_allocate(mempool_t * pool);

/* Deallocate a block of memory back to the memory pool
 * Argument 1 is a pointer to a memory pool
 * Argument 2 is a pointer to a block of memory
 */
void pool_deallocate(mempool_t * pool, void * block);

/* Initiate the memory pool by allocating it blocks of memory from the static memory pool
 * Argument 1 is a pointer to a memory pool
 * Argument 2 is the requested size of memory blocks
 * Argument 3 is the requested number of memory blocks
 * Memory blocks are aligned to 8 bits
 */
void pool_init(mempool_t * pool, size_t blocksize, size_t blocks);


#define pool_add pool_deallocate

#endif /* MEMPOOL_H */
