#include <arch/mp.h>
#include <assert.h>
#include <lk/trace.h>
#include <lk/err.h>
#include <platform/interrupts.h>
#include <arch/ops.h>

/* bcm28xx has a weird custom interrupt controller for MP */
extern void bcm28xx_send_ipi(uint32_t irq, uint32_t cpu_mask);

#define LOCAL_TRACE 0

#define GIC_IPI_BASE (14)

status_t arch_mp_send_ipi(mp_cpu_mask_t target, mp_ipi_t ipi) {
    LTRACEF("target 0x%x, ipi %u\n", target, ipi);

    /* filter out targets outside of the range of cpus we care about */
    target &= ((1UL << SMP_MAX_CPUS) - 1);
    if (target != 0) {
        bcm28xx_send_ipi(ipi, target);
    }

    return NO_ERROR;
}

static enum handler_return arm_ipi_generic_handler(void *arg) {
    LTRACEF("cpu %u, arg %p\n", arch_curr_cpu_num(), arg);

    return INT_NO_RESCHEDULE;
}

static enum handler_return arm_ipi_reschedule_handler(void *arg) {
    LTRACEF("cpu %u, arg %p\n", arch_curr_cpu_num(), arg);

    return mp_mbx_reschedule_irq();
}

void arch_mp_init_percpu(void) {
    register_int_handler(MP_IPI_GENERIC + GIC_IPI_BASE, &arm_ipi_generic_handler, 0);
    register_int_handler(MP_IPI_RESCHEDULE + GIC_IPI_BASE, &arm_ipi_reschedule_handler, 0);

    //unmask_interrupt(MP_IPI_GENERIC + GIC_IPI_BASE);
    //unmask_interrupt(MP_IPI_RESCHEDULE + GIC_IPI_BASE);
}

