#include <machine/asm.h>
#include <machine/defines.h>

#define IMAGE_OFFSET 0 /* Image offset from start of 2 MiB page */
#define IMAGE_SIZE __kernel_size
#define IMAGE_FLAGS 2 /* Kernel is little endian, page size is 4KiB */

.section .boot.text, "ax"
ENTRY(_start)
    b       arm64_reset
    .long   0
    .quad   IMAGE_OFFSET
    .quad   IMAGE_SIZE
    .quad   IMAGE_FLAGS
    .quad   0
    .quad   0
    .quad   0
    .long   0x644d5241 /* Magic "ARM\x64" */
    .long   0

arm64_reset:
    /* Get CPU number. */
    mrs     x3, MPIDR_EL1
    and     x3, x3, #3
    cmp     x3, #0
    bne     .

    /* Save pointer to dtb. */
    mov     x19, x0

    /* Setup initial stack. */
    adr     x3, __boot_stack_end
    mov     sp, x3

    bl arm64_elX_to_el1

    

    mov x30, #0xcafe
    b .



END(_start)





.global  __boot_stack
.global __boot_stack_end

    .align  4
__boot_stack:
    .space  KERNEL_STACK_SIZE
__boot_stack_end:
