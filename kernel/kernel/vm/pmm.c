#include <kernel/vm.h>
#include <assert.h>
#include <kernel/mutex.h>
#include <lk/err.h>
#include <lk/list.h>
#include <lk/pow2.h>
#include <stdlib.h>
#include <string.h>

#include "vm_priv.h"

static struct list_node arena_list = LIST_INITIAL_VALUE(arena_list);
static mutex_t lock = MUTEX_INITIAL_VALUE(lock);

#define PAGE_BELONGS_TO_ARENA(page, arena) \
    (((uintptr_t)(page) >= (uintptr_t)(arena)->page_array) && \
     ((uintptr_t)(page) < ((uintptr_t)(arena)->page_array + (arena)->size / PAGE_SIZE * sizeof(vm_page_t))))

#define PAGE_ADDRESS_FROM_ARENA(page, arena) \
    (paddr_t)(((uintptr_t)page - (uintptr_t)(arena)->page_array) / sizeof(vm_page_t)) * PAGE_SIZE + (arena)->base;

#define ADDRESS_IN_ARENA(address, arena) \
    ((address) >= (arena)->base && (address) <= (arena)->base + (arena)->size - 1)

static inline bool page_is_free(const vm_page_t *page) {
    return !(page->flags & VM_PAGE_FLAG_NONFREE);
}

paddr_t vm_page_to_paddr(const vm_page_t *page) {
    pmm_arena_t *a;
    list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
        if (PAGE_BELONGS_TO_ARENA(page, a)) {
            return PAGE_ADDRESS_FROM_ARENA(page, a);
        }
    }
    return -1;
}

vm_page_t *paddr_to_vm_page(paddr_t addr) {
    pmm_arena_t *a;
    list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
        if (addr >= a->base && addr <= a->base + a->size - 1) {
            size_t index = (addr - a->base) / PAGE_SIZE;
            return &a->page_array[index];
        }
    }
    return NULL;
}

status_t pmm_add_arena(pmm_arena_t *arena) {
    dprintf(DEBUG, "arena %p name '%s' base 0x%lx size 0x%zx\n", arena, arena->name, arena->base, arena->size);

    DEBUG_ASSERT(IS_PAGE_ALIGNED(arena->base));
    DEBUG_ASSERT(IS_PAGE_ALIGNED(arena->size));
    DEBUG_ASSERT(arena->size > 0);

    /* walk the arena list and add arena based on priority order */
    pmm_arena_t *a;
    list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
        if (a->priority > arena->priority) {
            list_add_before(&a->node, &arena->node);
            goto done_add;
        }
    }

    /* walked off the end, add it to the end of the list */
    list_add_tail(&arena_list, &arena->node);

done_add:

    /* zero out some of the structure */
    arena->free_count = 0;
    list_initialize(&arena->free_list);

    /* allocate an array of pages to back this one */
    size_t page_count = arena->size / PAGE_SIZE;
    arena->page_array = boot_alloc_mem(page_count * sizeof(vm_page_t));

    /* initialize all of the pages */
    memset(arena->page_array, 0, page_count * sizeof(vm_page_t));

    /* add them to the free list */
    for (size_t i = 0; i < page_count; i++) {
        vm_page_t *p = &arena->page_array[i];

        list_add_tail(&arena->free_list, &p->node);

        arena->free_count++;
    }

    return NO_ERROR;
}

size_t pmm_alloc_pages(uint32_t count, struct list_node *list) {
    dprintf(DEBUG, "count %u\n", count);

    /* list must be initialized prior to calling this */
    DEBUG_ASSERT(list);

    uint32_t allocated = 0;
    if (count == 0)
        return 0;

    mutex_acquire(&lock);

    /* walk the arenas in order, allocating as many pages as we can from each */
    pmm_arena_t *a;
    list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
        while (allocated < count) {
            vm_page_t *page = list_remove_head_type(&a->free_list, vm_page_t, node);
            if (!page)
                goto done;

            a->free_count--;

            page->flags |= VM_PAGE_FLAG_NONFREE;
            list_add_tail(list, &page->node);

            allocated++;
        }
    }

done:
    mutex_release(&lock);
    return allocated;
}

vm_page_t *pmm_alloc_page(void) {
    struct list_node list = LIST_INITIAL_VALUE(list);

    size_t ret = pmm_alloc_pages(1, &list);
    if (ret == 0) {
        return NULL;
    }

    DEBUG_ASSERT(ret == 1);

    return list_peek_head_type(&list, vm_page_t, node);
}

size_t pmm_alloc_range(paddr_t address, uint32_t count, struct list_node *list) {
    dprintf(DEBUG, "address 0x%lx, count %u\n", address, count);

    DEBUG_ASSERT(list);

    uint32_t allocated = 0;
    if (count == 0)
        return 0;

    address = ROUNDDOWN(address, PAGE_SIZE);

    mutex_acquire(&lock);

    /* walk through the arenas, looking to see if the physical page belongs to it */
    pmm_arena_t *a;
    list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
        while (allocated < count && ADDRESS_IN_ARENA(address, a)) {
            size_t index = (address - a->base) / PAGE_SIZE;

            DEBUG_ASSERT(index < a->size / PAGE_SIZE);

            vm_page_t *page = &a->page_array[index];
            if (page->flags & VM_PAGE_FLAG_NONFREE) {
                /* we hit an allocated page */
                break;
            }

            DEBUG_ASSERT(list_in_list(&page->node));

            list_delete(&page->node);
            page->flags |= VM_PAGE_FLAG_NONFREE;
            list_add_tail(list, &page->node);

            a->free_count--;
            allocated++;
            address += PAGE_SIZE;
        }

        if (allocated == count)
            break;
    }

    mutex_release(&lock);
    return allocated;
}

size_t pmm_free(struct list_node *list) {
    dprintf(DEBUG, "list %p\n", list);

    DEBUG_ASSERT(list);

    mutex_acquire(&lock);

    size_t count = 0;
    while (!list_is_empty(list)) {
        vm_page_t *page = list_remove_head_type(list, vm_page_t, node);

        DEBUG_ASSERT(!list_in_list(&page->node));
        DEBUG_ASSERT(page->flags & VM_PAGE_FLAG_NONFREE);

        /* see which arena this page belongs to and add it */
        pmm_arena_t *a;
        list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
            if (PAGE_BELONGS_TO_ARENA(page, a)) {
                page->flags &= ~VM_PAGE_FLAG_NONFREE;

                list_add_head(&a->free_list, &page->node);
                a->free_count++;
                count++;
                break;
            }
        }
    }

    mutex_release(&lock);
    return count;
}

size_t pmm_free_page(vm_page_t *page) {
    struct list_node list;
    list_initialize(&list);

    list_add_head(&list, &page->node);

    return pmm_free(&list);
}

/* physically allocate a run from arenas marked as KMAP */
void *pmm_alloc_kpages(uint32_t count, struct list_node *list) {
    dprintf(DEBUG, "count %u\n", count);

    /* fast path for single page */
    if (count == 1) {
        vm_page_t *p = pmm_alloc_page();
        if (!p) {
            return NULL;
        }

        return paddr_to_kvaddr(vm_page_to_paddr(p));
    }

    paddr_t pa;
    size_t alloc_count = pmm_alloc_contiguous(count, PAGE_SIZE_SHIFT, &pa, list);
    if (alloc_count == 0)
        return NULL;

    return paddr_to_kvaddr(pa);
}

size_t pmm_free_kpages(void *_ptr, uint32_t count) {
    dprintf(DEBUG, "ptr %p, count %u\n", _ptr, count);

    uint8_t *ptr = (uint8_t *)_ptr;

    struct list_node list;
    list_initialize(&list);

    while (count > 0) {
        vm_page_t *p = paddr_to_vm_page(vaddr_to_paddr(ptr));
        if (p) {
            list_add_tail(&list, &p->node);
        }

        ptr += PAGE_SIZE;
        count--;
    }

    return pmm_free(&list);
}

size_t pmm_alloc_contiguous(uint32_t count, uint8_t alignment_log2, paddr_t *pa, struct list_node *list) {
    dprintf(DEBUG, "count %u, align %u\n", count, alignment_log2);

    if (count == 0)
        return 0;
    if (alignment_log2 < PAGE_SIZE_SHIFT)
        alignment_log2 = PAGE_SIZE_SHIFT;

    mutex_acquire(&lock);

    pmm_arena_t *a;
    list_for_every_entry(&arena_list, a, pmm_arena_t, node) {
        // XXX make this a flag to only search kmap?
        if (a->flags & PMM_ARENA_FLAG_KMAP) {
            /* walk the list starting at alignment boundaries.
             * calculate the starting offset into this arena, based on the
             * base address of the arena to handle the case where the arena
             * is not aligned on the same boundary requested.
             */
            paddr_t rounded_base = ROUNDUP(a->base, 1UL << alignment_log2);
            if (rounded_base < a->base || rounded_base > a->base + a->size - 1)
                continue;

            uint32_t aligned_offset = (rounded_base - a->base) / PAGE_SIZE;
            uint32_t start = aligned_offset;
            dprintf(DEBUG, "starting search at aligned offset %u\n", start);
            dprintf(DEBUG, "arena base 0x%lx size %zu\n", a->base, a->size);

retry:
            /* search while we're still within the arena and have a chance of finding a slot
               (start + count < end of arena) */
            while ((start < a->size / PAGE_SIZE) &&
                    ((start + count) <= a->size / PAGE_SIZE)) {
                vm_page_t *p = &a->page_array[start];
                for (uint32_t i = 0; i < count; i++) {
                    if (p->flags & VM_PAGE_FLAG_NONFREE) {
                        /* this run is broken, break out of the inner loop.
                         * start over at the next alignment boundary
                         */
                        start = ROUNDUP(start - aligned_offset + i + 1, 1UL << (alignment_log2 - PAGE_SIZE_SHIFT)) + aligned_offset;
                        goto retry;
                    }
                    p++;
                }

                /* we found a run */
                dprintf(DEBUG, "found run from pn %u to %u\n", start, start + count);

                /* remove the pages from the run out of the free list */
                for (uint32_t i = start; i < start + count; i++) {
                    p = &a->page_array[i];
                    DEBUG_ASSERT(!(p->flags & VM_PAGE_FLAG_NONFREE));
                    DEBUG_ASSERT(list_in_list(&p->node));

                    list_delete(&p->node);
                    p->flags |= VM_PAGE_FLAG_NONFREE;
                    a->free_count--;

                    if (list)
                        list_add_tail(list, &p->node);
                }

                if (pa)
                    *pa = a->base + start * PAGE_SIZE;

                mutex_release(&lock);

                return count;
            }
        }
    }

    mutex_release(&lock);

    dprintf(ERROR, "couldn't find run\n");
    return 0;
}

static void dump_page(const vm_page_t *page) {
    printf("page %p: address 0x%lx flags 0x%x\n", page, vm_page_to_paddr(page), page->flags);
}

static void dump_arena(const pmm_arena_t *arena, bool dump_pages) {
    printf("arena %p: name '%s' base 0x%lx size 0x%zx priority %u flags 0x%x\n",
           arena, arena->name, arena->base, arena->size, arena->priority, arena->flags);
    printf("\tpage_array %p, free_count %zu\n",
           arena->page_array, arena->free_count);

    /* dump all of the pages */
    if (dump_pages) {
        for (size_t i = 0; i < arena->size / PAGE_SIZE; i++) {
            dump_page(&arena->page_array[i]);
        }
    }

    /* dump the free pages */
    printf("\tfree ranges:\n");
    ssize_t last = -1;
    for (size_t i = 0; i < arena->size / PAGE_SIZE; i++) {
        if (page_is_free(&arena->page_array[i])) {
            if (last == -1) {
                last = i;
            }
        } else {
            if (last != -1) {
                printf("\t\t0x%lx - 0x%lx\n", arena->base + last * PAGE_SIZE, arena->base + i * PAGE_SIZE);
            }
            last = -1;
        }
    }

    if (last != -1) {
        printf("\t\t0x%lx - 0x%lx\n",  arena->base + last * PAGE_SIZE, arena->base + arena->size);
    }
}
