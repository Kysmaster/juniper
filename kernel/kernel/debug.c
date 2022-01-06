#include <kernel/mp.h>
#include <kernel/thread.h>
#include <lk/debug.h>
#include <lk/err.h>
#include <stdio.h>
#include <kernel/spinlock.h>
#include <dev/uart.h>
#include <string.h>

/* global uart console io handle */
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

static spin_lock_t debug_lock = SPIN_LOCK_INITIAL_VALUE;

int dprintf(uint8_t level, const char *fmt, ...) {
    va_list ap;
    int err;
	char buffer[256];
	extern size_t uptime_raw;
	lk_time_t timestamp = uptime_raw;
	
	switch (level) {
	case 1:
		snprintf(buffer, sizeof(buffer), "[ %09d ] \e[36m[info]\e[37m: ", timestamp);
		break;
	case 2:
		snprintf(buffer, sizeof(buffer), "[ %09d ] \e[31m[ERROR]\e[37m: ", timestamp);
		break;
	case 3:
		snprintf(buffer, sizeof(buffer), "[ %09d ] \e[31m[PANIC]\e[37m: ", timestamp);
		break;
	default:
		if(SHOW_DEBUG) {
			snprintf(buffer, sizeof(buffer), "[ %09d ] \e[36m[DEBUG]\e[37m: ", timestamp);
		} else {
   			return err;
		}
		break;
	}

	spin_lock_saved_state_t state;
    spin_lock_save(&debug_lock, &state, SPIN_LOCK_FLAG_INTERRUPTS);
	
    va_start(ap, fmt);
	fputs(buffer, &uart_out);
    err = vfprintf(&uart_out, fmt, ap);
    va_end(ap);

	spin_unlock_restore(&debug_lock, state, SPIN_LOCK_FLAG_INTERRUPTS);
    return err;
}
