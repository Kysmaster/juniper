/*
 * Copyright (c) 2008-2014 Travis Geiselbrecht
 *
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT
 */

/**
 * @defgroup debug  Debug
 * @{
 */

/**
 * @file
 * @brief  Debug console functions.
 */

#include <kernel/debug.h>

#include <kernel/mp.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <lk/console_cmd.h>
#include <lk/debug.h>
#include <lk/err.h>
#include <platform.h>
#include <stdio.h>
#include <kernel/spinlock.h>

static int cmd_threads(int argc, const console_cmd_args *argv);
static int cmd_threads_panic(int argc, const console_cmd_args *argv);
static int cmd_threadstats(int argc, const console_cmd_args *argv);
static int cmd_threadload(int argc, const console_cmd_args *argv);
static int cmd_kevlog(int argc, const console_cmd_args *argv);

STATIC_COMMAND_START
#if LK_DEBUGLEVEL > 1
STATIC_COMMAND("threads", "list kernel threads", &cmd_threads)
STATIC_COMMAND_MASKED("threads", "list kernel threads", &cmd_threads_panic, CMD_AVAIL_PANIC)
#endif
#if THREAD_STATS
STATIC_COMMAND("threadstats", "thread level statistics", &cmd_threadstats)
STATIC_COMMAND("threadload", "toggle thread load display", &cmd_threadload)
#endif
#if WITH_KERNEL_EVLOG
STATIC_COMMAND_MASKED("kevlog", "dump kernel event log", &cmd_kevlog, CMD_AVAIL_ALWAYS)
#endif
STATIC_COMMAND_END(kernel);

#if LK_DEBUGLEVEL > 1
static int cmd_threads(int argc, const console_cmd_args *argv) {
    printf("thread list:\n");
    dump_all_threads();

    return 0;
}

static int cmd_threads_panic(int argc, const console_cmd_args *argv) {
    /* call the unsafe version of the thread dump routine since the
     * thread lock may be held at crash time.
     */
    printf("thread list:\n");
    dump_all_threads_unlocked();

    return 0;
}
#endif

#if THREAD_STATS
static int cmd_threadstats(int argc, const console_cmd_args *argv) {
    for (uint i = 0; i < SMP_MAX_CPUS; i++) {
        if (!mp_is_cpu_active(i))
            continue;

        printf("thread stats (cpu %d):\n", i);
        printf("\ttotal idle time: %lld\n", thread_stats[i].idle_time);
        printf("\ttotal busy time: %lld\n", current_time_hires() - thread_stats[i].idle_time);
        printf("\treschedules: %lu\n", thread_stats[i].reschedules);
        printf("\treschedule_ipis: %lu\n", thread_stats[i].reschedule_ipis);
        printf("\tcontext_switches: %lu\n", thread_stats[i].context_switches);
        printf("\tpreempts: %lu\n", thread_stats[i].preempts);
        printf("\tyields: %lu\n", thread_stats[i].yields);
        printf("\tinterrupts: %lu\n", thread_stats[i].interrupts);
        printf("\ttimer interrupts: %lu\n", thread_stats[i].timer_ints);
        printf("\ttimers: %lu\n", thread_stats[i].timers);
    }

    return 0;
}

static enum handler_return threadload(struct timer *t, lk_time_t now, void *arg) {
    static struct thread_stats old_stats[SMP_MAX_CPUS];
    static lk_bigtime_t last_idle_time[SMP_MAX_CPUS];

    for (uint i = 0; i < SMP_MAX_CPUS; i++) {
        /* dont display time for inactiv cpus */
        if (!mp_is_cpu_active(i))
            continue;

        lk_bigtime_t idle_time = thread_stats[i].idle_time;

        /* if the cpu is currently idle, add the time since it went idle up until now to the idle counter */
        bool is_idle = !!mp_is_cpu_idle(i);
        if (is_idle) {
            idle_time += current_time_hires() - thread_stats[i].last_idle_timestamp;
        }

        lk_bigtime_t delta_time = idle_time - last_idle_time[i];
        lk_bigtime_t busy_time = 1000000ULL - (delta_time > 1000000ULL ? 1000000ULL : delta_time);
        uint busypercent = (busy_time * 10000) / (1000000);

        printf("cpu %u LOAD: "
               "%u.%02u%%, "
               "cs %lu, "
               "pmpts %lu, "
               "rs_ipis %lu, "
               "ints %lu, "
               "tmr ints %lu, "
               "tmrs %lu\n",
               i,
               busypercent / 100, busypercent % 100,
               thread_stats[i].context_switches - old_stats[i].context_switches,
               thread_stats[i].preempts - old_stats[i].preempts,
               thread_stats[i].reschedule_ipis - old_stats[i].reschedule_ipis,
               thread_stats[i].interrupts - old_stats[i].interrupts,
               thread_stats[i].timer_ints - old_stats[i].timer_ints,
               thread_stats[i].timers - old_stats[i].timers);

        old_stats[i] = thread_stats[i];
        last_idle_time[i] = idle_time;
    }

    return INT_NO_RESCHEDULE;
}

static int cmd_threadload(int argc, const console_cmd_args *argv) {
    static bool showthreadload = false;
    static timer_t tltimer;

    if (showthreadload == false) {
        // start the display
        timer_initialize(&tltimer);
        timer_set_periodic(&tltimer, 1000, &threadload, NULL);
        showthreadload = true;
    } else {
        timer_cancel(&tltimer);
        showthreadload = false;
    }

    return 0;
}

#endif // THREAD_STATS

// TODO
size_t uart_write(io_handle_t *io, const char *str, size_t len);
size_t uart_read(io_handle_t *io, const char *str, size_t len);

extern spin_lock_t printf_lock;

/* global console io handle */
static const io_handle_hooks_t uart_io_hooks = {
    .write  = uart_write,
    .read   = uart_read,
};

io_handle_t uart_io = {
	.magic = 0xCAFEBABE, 
	.hooks = &uart_io_hooks, 
};

FILE uart_out = {
	.io = &uart_io,
};

int dprintf(uint8_t level, const char *fmt, ...) {
	spin_lock(&printf_lock);
    va_list ap;
    int err;

	switch (level) {
	case 1:
		fputs("\e[36minfo\e[37m: ", &uart_out);
		break;
	case 2:
		fputs("\e[31mERROR\e[37m: ", &uart_out);
		break;
	case 3:
		fputs("\e[31mPANIC\e[37m: ", &uart_out);
		break;
	default:
		if(SHOW_DEBUG) {
			fputs("\e[36mDEBUG\e[37m: ", &uart_out);
		} else {
			spin_unlock(&printf_lock);
   			return err;
		}
		
		break;
	}
	

    va_start(ap, fmt);
    err = vfprintf(&uart_out, fmt, ap);
    va_end(ap);

	spin_unlock(&printf_lock);
    return err;
}
