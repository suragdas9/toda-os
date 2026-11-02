/**
 * Toda OS — Stage 1 Bootloader
 * Target: ARM64 (AArch64)
 *
 * This is the FIRST code that runs when a Toda OS phone powers on.
 * Pure Assembly. No libraries. No OS. Just raw hardware.
 *
 * Copyright (c) Toda OS Project
 */

.section ".text.boot"
.global _start

_start:
    /* -----------------------------------------------
     * Step 1: Only CPU core 0 continues
     * All other cores sleep — we boot single threaded
     * ----------------------------------------------- */
    mrs     x0, mpidr_el1          /* Read Multiprocessor Affinity Register */
    and     x0, x0, #3             /* Isolate core ID bits */
    cbnz    x0, cpu_idle           /* If not core 0 → sleep */

    /* -----------------------------------------------
     * Step 2: Set Exception Level to EL1
     * Phones boot at EL2 (hypervisor level)
     * We drop to EL1 (kernel level)
     * ----------------------------------------------- */
    mrs     x0, CurrentEL
    lsr     x0, x0, #2
    cmp     x0, #2
    bne     setup_stack            /* Already at EL1, skip */

    /* Drop from EL2 to EL1 */
    mov     x0, #0x3c5
    msr     spsr_el2, x0
    adr     x0, setup_stack
    msr     elr_el2, x0
    eret                           /* Exception return → drops to EL1 */

setup_stack:
    /* -----------------------------------------------
     * Step 3: Set up stack pointer
     * Stack grows DOWN from _start address
     * ----------------------------------------------- */
    ldr     x0, =_start
    mov     sp, x0

    /* -----------------------------------------------
     * Step 4: Zero out BSS section
     * BSS = uninitialized global variables in C
     * Must be zero before kernel_main runs
     * ----------------------------------------------- */
    ldr     x0, =__bss_start
    ldr     x1, =__bss_end
    mov     x2, #0

bss_clear_loop:
    cmp     x0, x1
    bge     bss_clear_done
    str     x2, [x0], #8
    b       bss_clear_loop

bss_clear_done:
    /* -----------------------------------------------
     * Step 5: Jump to Toda OS Kernel (C code)
     * The real OS starts here
     * ----------------------------------------------- */
    bl      kernel_main

    /* -----------------------------------------------
     * kernel_main should NEVER return
     * If it does — something is catastrophically wrong
     * Just freeze the CPU
     * ----------------------------------------------- */

cpu_idle:
    wfe                            /* ARM low-power Wait For Event */
    b       cpu_idle               /* Loop forever */

