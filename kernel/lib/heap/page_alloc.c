#include <lib/page_alloc.h>
#include <lk/debug.h>
#include <assert.h>
#include <string.h>
#include <kernel/vm.h>

/* A simple page-aligned wrapper around the pmm or novm implementation of
 * the underlying physical page allocator. Used by system heaps or any
 * other user that wants pages of memory but doesn't want to use LK
 * specific apis.
 */

#if WITH_STATIC_HEAP

#error "fix static heap post page allocator and novm stuff"

#if !defined(HEAP_START) || !defined(HEAP_LEN)
#error WITH_STATIC_HEAP set but no HEAP_START or HEAP_LEN defined
#endif

#endif

void *page_alloc(size_t pages, int arena) {
    void *result = pmm_alloc_kpages(pages, NULL);
    return result;
}

void page_free(void *ptr, size_t pages) {
    DEBUG_ASSERT(IS_PAGE_ALIGNED((uintptr_t)ptr));

    pmm_free_kpages(ptr, pages);
}

int page_get_arenas(struct page_range *ranges, int number_of_ranges) {
    ranges[0].address = kvaddr_get_range(&ranges[0].size);
    return 1;
}

void *page_first_alloc(size_t *size_return) {
    *size_return = PAGE_SIZE;
    return page_alloc(1, PAGE_ALLOC_ANY_ARENA);
}
