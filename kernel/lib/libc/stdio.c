#include <kernel/kernel_struct.h>
#include <lk/debug.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <kernel/spinlock.h>

size_t platform_write(const char *str, size_t len);
FILE __stdio_FILEs[3] = {
    &console_io, /* stdin */
    &console_io, /* stdout */
    &console_io, /* stderr */
};

spin_lock_t printf_lock = SPIN_LOCK_INITIAL_VALUE;

#undef DEFINE_STDIO_DESC

int fputc(int _c, FILE *fp) {
    unsigned char c = _c;
    return io_write(fp->io, (char *)&c, 1);
}

int putchar(int c) {
    return fputc(c, stdout);
}

int puts(const char *str) {
    int err = fputs(str, stdout);
    if (err >= 0)
        err = fputc('\n', stdout);
    return err;
}

int fputs(const char *s, FILE *fp) {
    size_t len = strlen(s);

    return io_write(fp->io, s, len);
}

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *fp) {
    size_t bytes_written;

    if (size == 0 || count == 0)
        return 0;

    // fast path for size == 1
    if (likely(size == 1)) {
        return io_write(fp->io, ptr, count);
    }

    bytes_written = io_write(fp->io, ptr, size * count);
    return bytes_written / size;
}

int getc(FILE *fp) {
    char c;
    ssize_t ret = io_read(fp->io, &c, sizeof(c));

    return (ret > 0) ? c : ret;
}

int getchar(void) {
    return getc(stdin);
}

int _fprintf_output_func(const char *str, size_t len, void *state) {
    FILE *fp = (FILE *)state;

    return io_write(fp->io, str, len);
}

int vfprintf(FILE *fp, const char *fmt, va_list ap) {
    return _printf_engine(&_fprintf_output_func, (void *)fp, fmt, ap);
}

int fprintf(FILE *fp, const char *fmt, ...) {
    va_list ap;
    int err;

    va_start(ap, fmt);
    err = vfprintf(fp, fmt, ap);
    va_end(ap);
    return err;
}

int printf(const char *fmt, ...) {
    va_list ap;
    int err;

	spin_lock_saved_state_t state;
	spin_lock_save(&printf_lock, &state, SPIN_LOCK_FLAG_INTERRUPTS);

    va_start(ap, fmt);
    err = vfprintf(stdout, fmt, ap);
    va_end(ap);

	spin_unlock_restore(&printf_lock, state, SPIN_LOCK_FLAG_INTERRUPTS);

    return err;
}

int vprintf(const char *fmt, va_list ap) {
    return vfprintf(stdout, fmt, ap);
}
