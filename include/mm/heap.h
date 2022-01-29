#pragma once

#include <stddef.h>
#include <sys/types.h>
#include <lk/compiler.h>

struct miniheap_stats {
    void *heap_start;
    size_t heap_len;
    size_t heap_free;
    size_t heap_max_chunk;
    size_t heap_low_watermark;
};

void miniheap_get_stats(struct miniheap_stats *ptr);

void *miniheap_alloc(size_t, unsigned int alignment);
void *miniheap_realloc(void *, size_t);
void miniheap_free(void *);

void miniheap_init(void *ptr, size_t len);
void miniheap_dump(void);
void miniheap_trim(void);

/* standard heap definitions */
void *malloc(size_t size) __MALLOC;
void *memalign(size_t boundary, size_t size) __MALLOC;
void *calloc(size_t count, size_t size) __MALLOC;
void *realloc(void *ptr, size_t size) __MALLOC;
void free(void *ptr);

void heap_init(void);

/* critical section time delayed free */
void heap_delayed_free(void *);

/* tell the heap to return any free pages it can find */
void heap_trim(void);
