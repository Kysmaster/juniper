#include <lk/debug.h>
#include <lk/err.h>
#include <lk/compiler.h>
#include <lk/console_cmd.h>
#include <platform.h>
#include <kernel/thread.h>
#include <stdio.h>

#if WITH_LIB_CONSOLE
#include <lib/console.h>
#endif

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

static int cmd_reboot(int argc, const console_cmd_args *argv) {
    platform_halt(HALT_ACTION_REBOOT, HALT_REASON_SW_RESET);
    return 0;
}

static int cmd_poweroff(int argc, const console_cmd_args *argv) {
    platform_halt(HALT_ACTION_SHUTDOWN, HALT_REASON_SW_RESET);
    return 0;
}

STATIC_COMMAND_START
#if LK_DEBUGLEVEL > 1
STATIC_COMMAND("reboot", "soft reset", &cmd_reboot)
STATIC_COMMAND("poweroff", "powerdown", &cmd_poweroff)
#endif
STATIC_COMMAND_END(platform_power);
