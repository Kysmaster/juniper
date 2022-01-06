#include <kernel/vm.h>
#include <arch/mmu.h>
#include <kernel/thread.h>
#include <lk/debug.h>
#include <lk/err.h>
#include <lk/init.h>
#include <string.h>

#include "vm_priv.h"

extern int _start;
extern int _end;

/* mark the physical pages backing a range of virtual as in use.
 * allocate the physical pages and throw them away */
static void mark_pages_in_use(vaddr_t va, size_t len) {
    dprintf(DEBUG, "va 0x%lx, len 0x%zx\n", va, len);

    struct list_node list;
    list_initialize(&list);

    /* make sure we are inclusive of all of the pages in the address range */
    len = PAGE_ALIGN(len + (va & (PAGE_SIZE - 1)));
    va = ROUNDDOWN(va, PAGE_SIZE);

    dprintf(DEBUG, "aligned va 0x%lx, len 0x%zx\n", va, len);

    for (size_t offset = 0; offset < len; offset += PAGE_SIZE) {
        uint32_t flags;
        paddr_t pa;

        status_t err = arch_mmu_query(&vmm_get_kernel_aspace()->arch_aspace, va + offset, &pa, &flags);
        if (err >= 0) {
            //dprintf(DEBUG, "va 0x%x, pa 0x%x, flags 0x%x, err %d\n", va + offset, pa, flags, err);

            /* alloate the range, throw the results away */
            pmm_alloc_range(pa, 1, &list);
        } else {
            panic("Could not find pa for va 0x%lx\n", va);
        }
    }
}

void vm_init_preheap(void) {
    /* allow the vmm a shot at initializing some of its data structures */
    vmm_init_preheap();

    /* mark all of the kernel pages in use */
    dprintf(INFO, "marking all kernel pages as used\n");
    mark_pages_in_use((vaddr_t)&_start, ((uintptr_t)&_end - (uintptr_t)&_start));

    /* mark the physical pages used by the boot time allocator */
    if (boot_alloc_end != boot_alloc_start) {
        dprintf(INFO, "marking boot alloc used from 0x%lx to 0x%lx\n", boot_alloc_start, boot_alloc_end);

        mark_pages_in_use(boot_alloc_start, boot_alloc_end - boot_alloc_start);
    }
}

void vm_init_postheap(void) {
    vmm_init();

    /* create vmm regions to cover what is already there from the initial mapping table */
    struct mmu_initial_mapping *map = mmu_initial_mappings;
    while (map->size > 0) {
        if (!(map->flags & MMU_INITIAL_MAPPING_TEMPORARY)) {
            vmm_reserve_space(vmm_get_kernel_aspace(), map->name, map->size, map->virt);
        }

        map++;
    }
}

void *kvaddr_get_range(size_t *size_return) {
    *size_return = mmu_initial_mappings->size;
    return (void *)mmu_initial_mappings->virt;
}

void *paddr_to_kvaddr(paddr_t pa) {
    /* slow path to do reverse lookup */
    struct mmu_initial_mapping *map = mmu_initial_mappings;
    while (map->size > 0) {
        if (!(map->flags & MMU_INITIAL_MAPPING_TEMPORARY) &&
                pa >= map->phys &&
                pa <= map->phys + map->size - 1) {
            return (void *)(map->virt + (pa - map->phys));
        }
        map++;
    }
    return NULL;
}

paddr_t vaddr_to_paddr(void *ptr) {
    vmm_aspace_t *aspace = vaddr_to_aspace(ptr);
    if (!aspace)
        return (paddr_t)NULL;

    paddr_t pa;
    status_t rc = arch_mmu_query(&aspace->arch_aspace, (vaddr_t)ptr, &pa, NULL);
    if (rc)
        return (paddr_t)NULL;

    return pa;
}

vmm_aspace_t *vaddr_to_aspace(void *ptr) {
    if (is_kernel_address((vaddr_t)ptr)) {
        return vmm_get_kernel_aspace();
    } else if (is_user_address((vaddr_t)ptr)) {
        return get_current_thread()->aspace;
    } else {
        return NULL;
    }
}
