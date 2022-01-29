#pragma once

#include <sys/types.h>

/* Routines implemented by the platform or system specific interrupt controller
 * to allow for installation and masking/unmask of interrupt vectors.
 *
 * Some platforms do not allow for dynamic registration.
 */
status_t mask_interrupt(unsigned int vector);
status_t unmask_interrupt(unsigned int vector);

typedef enum handler_return (*int_handler)(void *arg);
void register_int_handler(unsigned int vector, int_handler handler, void *arg);

/* Register a MSI interrupt handler. Basically the same as register_int_handler, except
 * interrupt controller may have additional setup.
 */
void register_int_handler_msi(unsigned int vector, int_handler handler, void *arg, bool edge);

/* Allocate a run of interrupts with alignment log2 in a platform specific way.
 * Used for PCI MSI and possibly other use cases.
 */
status_t platform_allocate_interrupts(size_t count, uint32_t align_log2, unsigned int *vector);

/* Map the incoming interrupt line number from the pci bus config to raw
 * vector number, usable in the above apis.
 */
status_t platform_pci_int_to_vector(unsigned int pci_int, unsigned int *vector);
