#pragma once

#define __nonnull(x) __attribute__((__nonnull__(x)))
#define __section(s) __attribute__((__section__(s)))

/* Attribute macros for boot/wired functions/data */
#define __boot_text __section(".boot.text")
#define __boot_data __section(".boot.data")