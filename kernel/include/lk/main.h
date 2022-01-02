#pragma once

#include <lk/compiler.h>
#include <sys/types.h>

void kernel_main(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3) __NO_RETURN __EXTERNALLY_VISIBLE;
void lk_secondary_cpu_entry(void);
void lk_init_secondary_cpus(uint32_t secondary_cpu_count);
