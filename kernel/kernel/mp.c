#include <kernel/mp.h>
#include <arch/atomic.h>
#include <arch/mp.h>
#include <assert.h>
#include <kernel/spinlock.h>
#include <lk/debug.h>
#include <lk/trace.h>
#include <stdlib.h>

#define LOCAL_TRACE 0

/* a global state structure, aligned on cpu cache line to minimize aliasing */
struct mp_state mp __CPU_ALIGN;

void mp_init(void) {
}

void mp_reschedule(mp_cpu_mask_t target, uint32_t flags) {
    uint32_t local_cpu = arch_curr_cpu_num();

    LTRACEF("local %d, target 0x%x\n", local_cpu, target);

    /* mask out cpus that are not active and the local cpu */
    target &= mp.active_cpus;

    /* mask out cpus that are currently running realtime code */
    if ((flags & MP_RESCHEDULE_FLAG_REALTIME) == 0) {
        target &= ~mp.realtime_cpus;
    }
    target &= ~(1U << local_cpu);

    LTRACEF("local %d, post mask target now 0x%x\n", local_cpu, target);

    arch_mp_send_ipi(target, MP_IPI_RESCHEDULE);
}

void mp_set_curr_cpu_active(bool active) {
    atomic_or((volatile int *)&mp.active_cpus, 1U << arch_curr_cpu_num());
}

enum handler_return mp_mbx_reschedule_irq(void) {
    uint32_t cpu = arch_curr_cpu_num();

    LTRACEF("cpu %u\n", cpu);

    THREAD_STATS_INC(reschedule_ipis);

    return (mp.active_cpus & (1U << cpu)) ? INT_RESCHEDULE : INT_NO_RESCHEDULE;
}
