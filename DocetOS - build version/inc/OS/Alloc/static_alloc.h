#ifndef STATIC_ALLOC_H
#define STATIC_ALLOC_H
#include <stddef.h>
#include <stdint.h>

/* Allocate a number of bytes from the static memory pool, aligned to 8 bits
 * Argument is the requested number of bytes to allocate from the pool
 * Return a pointer to the beginning of the allocate block of memory
 */
void * static_alloc(size_t bytes);

#endif /* STATIC_ALLOC_H */
