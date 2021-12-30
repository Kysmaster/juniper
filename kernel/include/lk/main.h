#pragma once

#include <lk/compiler.h>
#include <sys/types.h>

__BEGIN_CDECLS

void kern_main(ulong arg0, ulong arg1, ulong arg2, ulong arg3) __NO_RETURN __EXTERNALLY_VISIBLE;
void lk_secondary_cpu_entry(void);
void lk_init_secondary_cpus(uint secondary_cpu_count);

__END_CDECLS
