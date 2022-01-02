#pragma once

#include <stdbool.h>
#include <sys/types.h>
#include <lib/io.h>

void uart_init(void);
void uart_init_early(void);

int uart_putc(int port, char c);
int uart_getc(int port, bool wait);
void uart_flush_tx(int port);
void uart_flush_rx(int port);
void uart_init_port(int port, uint32_t baud);

/* panic-time uart accessors, intended to be run with interrupts disabled */
int uart_pputc(int port, char c);
int uart_pgetc(int port);

size_t uart_write(io_handle_t *io, const char *str, size_t len);
size_t uart_read(io_handle_t *io, const char *str, size_t len);

