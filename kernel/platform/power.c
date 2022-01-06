#include <lk/debug.h>
#include <lk/err.h>
#include <lk/compiler.h>
#include <platform.h>
#include <kernel/thread.h>
#include <stdio.h>

/*
 * default implementations of these routines, if the platform code
 * chooses not to implement.
 */
__WEAK void platform_halt(platform_halt_action suggested_action,
                          platform_halt_reason reason) {
#if ENABLE_PANIC_SHELL
    if (reason == HALT_REASON_SW_PANIC && suggested_action == HALT_ACTION_HALT) {
        dprintf(ALWAYS, "CRASH: starting debug shell... (reason = %d)\n", reason);
        arch_disable_ints();
        panic_shell_start();
    }
#endif  // ENABLE_PANIC_SHELL

    dprintf(INFO, "HALT: spinning forever... (reason = %d)\n", reason);
    arch_disable_ints();
    for (;;)
        arch_idle();
}
