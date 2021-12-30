/*
 * Copyright (c) 2013-2015 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

/*
 * Main entry point to the OS. Initializes modules in order and creates
 * the default thread.
 */
#include <lk/main.h>

//#include <app.h>
#include <arch.h>
#include <kernel/init.h>
#include <kernel/mutex.h>
#include <kernel/novm.h>
#include <kernel/thread.h>
#include <lib/heap.h>
#include <lk/compiler.h>
#include <lk/debug.h>
#include <lk/init.h>
#include <platform.h>
#include <string.h>
#include <target.h>

/* saved boot arguments from whoever loaded the system */
ulong lk_boot_args[4];

volatile int node_boot_lock = 0;

extern void *__ctor_list;
extern void *__ctor_end;
extern int __bss_start;
extern int _end;

static thread_t *secondary_bootstrap_threads[SMP_MAX_CPUS - 1];
static uint secondary_bootstrap_thread_count;

static int bootstrap2(void *arg);

static void call_constructors(void) {
    void **ctor;

    ctor = &__ctor_list;
    while (ctor != &__ctor_end) {
        void (*func)(void);

        func = (void ( *)(void))*ctor;

        func();
        ctor++;
    }
}

/* called from arch code */
void kern_main(ulong arg0, ulong arg1, ulong arg2, ulong arg3) {
    lk_boot_args[0] = arg0;
    lk_boot_args[1] = arg1;
    lk_boot_args[2] = arg2;
    lk_boot_args[3] = arg3;

    // get us into some sort of thread context
    thread_init_early();

    // early arch stuff
    arch_early_init();

    // do any super early platform initialization
    platform_early_init();

    dprintf(INFO, "\nwelcome to lk/MP\n\n");

    dprintf(INFO, "boot args 0x%lx 0x%lx 0x%lx 0x%lx\n",
            lk_boot_args[0], lk_boot_args[1], lk_boot_args[2], lk_boot_args[3]);

    // bring up the kernel heap

	void vm_init_preheap();
	vm_init_preheap();
    dprintf(SPEW, "initializing heap\n");
    heap_init();
	void vm_init_postheap();
	vm_init_postheap();

    // deal with any static constructors
    dprintf(SPEW, "calling constructors\n");
    //call_constructors();

    // initialize the kernel
    kernel_init();


    // create a thread to complete system initialization
    dprintf(SPEW, "creating bootstrap completion thread\n");
    thread_t *t = thread_create("bootstrap2", &bootstrap2, NULL, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    thread_set_pinned_cpu(t, 0);
    thread_detach(t);
    thread_resume(t);

    // become the idle thread and enable interrupts to start the scheduler
    thread_become_idle();
}

static int bootstrap2(void *arg) {
    dprintf(SPEW, "top of bootstrap2()\n");

	dprintf(SPEW, "initializing arch\n");
    arch_init();

    dprintf(SPEW, "initializing platform\n");
    platform_init();

	printf("OK!\n");

	node_boot_lock = 1;

    return 0;
}

void lk_secondary_cpu_entry(void) {
    uint cpu = arch_curr_cpu_num();

    if (cpu > secondary_bootstrap_thread_count) {
        dprintf(CRITICAL, "Invalid secondary cpu num %d, SMP_MAX_CPUS %d, secondary_bootstrap_thread_count %d\n",
                cpu, SMP_MAX_CPUS, secondary_bootstrap_thread_count);
        return;
    }

    thread_secondary_cpu_init_early();
    thread_resume(secondary_bootstrap_threads[cpu - 1]);

    dprintf(SPEW, "entering scheduler on cpu %d\n", cpu);
    thread_secondary_cpu_entry();
}

static int secondary_cpu_bootstrap2(void *arg) {
    /* secondary cpu initialize from threading level up. 0 to threading was handled in arch */

    return 0;
}

void lk_init_secondary_cpus(uint secondary_cpu_count) {
    if (secondary_cpu_count >= SMP_MAX_CPUS) {
        dprintf(CRITICAL, "Invalid secondary_cpu_count %d, SMP_MAX_CPUS %d\n",
                secondary_cpu_count, SMP_MAX_CPUS);
        secondary_cpu_count = SMP_MAX_CPUS - 1;
    }
    for (uint i = 0; i < secondary_cpu_count; i++) {
        dprintf(SPEW, "creating bootstrap completion thread for cpu %d\n", i + 1);
        thread_t *t = thread_create("secondarybootstrap2",
                                    &secondary_cpu_bootstrap2, NULL,
                                    DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
        t->pinned_cpu = i + 1;
        thread_detach(t);
        secondary_bootstrap_threads[i] = t;
    }
    secondary_bootstrap_thread_count = secondary_cpu_count;
}
