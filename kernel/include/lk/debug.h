#pragma once

#include <lk/compiler.h>
#include <stddef.h>
#include <stdio.h>

#if !defined(LK_DEBUGLEVEL)
#define LK_DEBUGLEVEL 0
#endif

/* debug levels */
#define DEBUG 0
#define INFO 1
#define ERROR 2

#define SHOW_DEBUG 0
 

__BEGIN_CDECLS

#if !DISABLE_DEBUG_OUTPUT

/* Obtain the panic file descriptor. */
FILE *get_panic_fd(void);

/* dump memory */
void hexdump(const void *ptr, size_t len);
void hexdump8_ex(const void *ptr, size_t len, uint64_t disp_addr_start);

#else

/* Obtain the panic file descriptor. */
static inline FILE *get_panic_fd(void) { return NULL; }

/* dump memory */
static inline void hexdump(const void *ptr, size_t len) { }
static inline void hexdump8_ex(const void *ptr, size_t len, uint64_t disp_addr_start) { }

#endif /* DISABLE_DEBUG_OUTPUT */

static inline void hexdump8(const void *ptr, size_t len) {
    hexdump8_ex(ptr, len, (uint64_t)((addr_t)ptr));
}

int dprintf(uint8_t level, const char *fmt, ...);

/* systemwide halts */
void panic(const char *fmt, ...) __PRINTFLIKE(1, 2) __NO_RETURN;

#define PANIC_UNIMPLEMENTED panic("%s:%d unimplemented\n", __PRETTY_FUNCTION__, __LINE__)
#define PANIC_UNIMPLEMENTED_MSG(x...) panic("%s:%d unimplemented: %s\n", __PRETTY_FUNCTION__, __LINE__, x)

/* spin the cpu for a period of (short) time */
void spin(uint32_t usecs);

/* spin the cpu for a certain number of cpu cycles */
void spin_cycles(uint32_t usecs);

__END_CDECLS
