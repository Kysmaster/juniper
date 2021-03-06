#pragma once

#define MEMBASE 0x00000000
#define MEMSIZE 0x40000000
#define ARCH_DEFAULT_STACK_SIZE 4096
#define SMP_MAX_CPUS 4
#define SMP_CPU_CLUSTER_SHIFT 8
#define SMP_CPU_ID_BITS 24
#define KERNEL_ASPACE_BASE 0xffff000000000000
#define KERNEL_ASPACE_SIZE 0x0001000000000000
#define USER_ASPACE_BASE 0x0000000001000000
#define USER_ASPACE_SIZE 0x0000fffffe000000
#define KERNEL_BASE 0xffff000000000000
#define KERNEL_LOAD_OFFSET 0x00080000

#define SHIFT_4K        (12)
#define SHIFT_16K       (14)
#define SHIFT_64K       (16)

/* arm specific stuff */
#ifdef ARM64_LARGE_PAGESIZE_64K
#define PAGE_SIZE_SHIFT (SHIFT_64K)
#elif ARM64_LARGE_PAGESIZE_16K
#define PAGE_SIZE_SHIFT (SHIFT_16K)
#else
#define PAGE_SIZE_SHIFT (SHIFT_4K)
#endif
#define USER_PAGE_SIZE_SHIFT SHIFT_4K

#define PAGE_SIZE (1UL << PAGE_SIZE_SHIFT)
#define USER_PAGE_SIZE (1UL << USER_PAGE_SIZE_SHIFT)

#define CACHE_LINE 64