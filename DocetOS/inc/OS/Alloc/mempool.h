#ifndef MEMPOOL_H
#define MEMPOOL_H
#include <stddef.h>
#include "OS/Alloc/static_alloc.h"

struct mempool_item_s {
	struct mempool_item_s *next;
};
typedef struct mempool_item_s mempool_item_t;

typedef struct {
	mempool_item_t *head;
} mempool_t;

#define MEMPOOL_INITIALISER { .head = 0 }

void *pool_allocate(mempool_t *pool);
void pool_deallocate(mempool_t *pool, void *block);
void pool_init(mempool_t *pool, size_t blocksize, size_t blocks);


#define pool_add pool_deallocate

#endif /* MEMPOOL_H */
