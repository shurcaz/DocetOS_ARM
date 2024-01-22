#ifndef STATIC_ALLOC_H
#define STATIC_ALLOC_H
#include <stddef.h>
#include <stdint.h>

void * static_alloc(size_t bytes);

#define STATIC_ALLOC_ALIGNMENT 8U
#define ALIGNMENT_MASK ~(STATIC_ALLOC_ALIGNMENT - 1)

#endif /* STATIC_ALLOC_H */
