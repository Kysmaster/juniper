#pragma once

#include <stdint.h>
#include <kernel/spinlock.h>

struct kernel_struct {
	uint64_t boot_args[4];			/* saved boot arguments from whoever loaded the system */
	spin_lock_t printf_lock;		/* printf like functions spinlock */
	volatile int node_boot_locked;	/* infinite loop blockade in arm64_secondary_entry() */
};

extern struct kernel_struct kernel_struct;
