#include <kernel/debug.h>
#include <kernel/kernel_struct.h>
#include <kernel/mp.h>
#include <kernel/thread.h>
#include <kernel/timer.h>
#include <lk/console_cmd.h>
#include <lk/debug.h>
#include <lk/err.h>
#include <platform.h>
#include <stdio.h>
#include <kernel/spinlock.h>
#include <dev/uart.h>

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

int dprintf(uint8_t level, const char *fmt, ...) {
	spin_lock(&kernel_struct.printf_lock);
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
			spin_unlock(&kernel_struct.printf_lock);
   			return err;
		}
		
		break;
	}

    va_start(ap, fmt);
    err = vfprintf(&uart_out, fmt, ap);
    va_end(ap);

	spin_unlock(&kernel_struct.printf_lock);
    return err;
}
