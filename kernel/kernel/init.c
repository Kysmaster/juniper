#include <kernel/init.h>
#include <kernel/mp.h>
#include <kernel/port.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <lk/compiler.h>
#include <lk/debug.h>

void kernel_init(void) {
	// initialize the threading system
    dprintf(INFO, "initializing mp\n");
    mp_init();

    // initialize the threading system
    dprintf(INFO, "initializing threads\n");
    thread_init();

    // initialize kernel timers
    dprintf(INFO, "initializing timers\n");
    timer_init();

    // initialize ports
    dprintf(INFO, "initializing ports\n");
    port_init();
}

