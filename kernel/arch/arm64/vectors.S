/**
 * Toda OS — ARM64 Exception Vector Table & Context Switching
 *
 * Implements standard ARM64 16-entry exception table (VBAR_EL1)
 * and assembly register save/restore and context switch routines.
 *
 * Copyright (c) Toda OS Project
 */

.section ".text", "ax"

/* ============================================================
 * Context Save / Restore Macros
 * Frame size: 288 bytes (16-byte aligned)
 * ============================================================ */
.macro SAVE_ALL
    sub     sp, sp, #288
    stp     x0,  x1,  [sp, #0]
    stp     x2,  x3,  [sp, #16]
    stp     x4,  x5,  [sp, #32]
    stp     x6,  x7,  [sp, #48]
    stp     x8,  x9,  [sp, #64]
    stp     x10, x11, [sp, #80]
    stp     x12, x13, [sp, #96]
    stp     x14, x15, [sp, #112]
    stp     x16, x17, [sp, #128]
    stp     x18, x19, [sp, #144]
    stp     x20, x21, [sp, #160]
    stp     x22, x23, [sp, #176]
    stp     x24, x25, [sp, #192]
    stp     x26, x27, [sp, #208]
    stp     x28, x29, [sp, #224]
    str     x30,      [sp, #240]

    /* Save system exception registers */
    mrs     x0, elr_el1
    mrs     x1, spsr_el1
    stp     x0, x1,   [sp, #248]

    mrs     x0, esr_el1
    mrs     x1, far_el1
    stp     x0, x1,   [sp, #264]
.endm

.macro RESTORE_ALL
    /* Restore system exception registers */
    ldp     x0, x1,   [sp, #248]
    msr     elr_el1, x0
    msr     spsr_el1, x1

    /* Restore general purpose registers */
    ldp     x0,  x1,  [sp, #0]
    ldp     x2,  x3,  [sp, #16]
    ldp     x4,  x5,  [sp, #32]
    ldp     x6,  x7,  [sp, #48]
    ldp     x8,  x9,  [sp, #64]
    ldp     x10, x11, [sp, #80]
    ldp     x12, x13, [sp, #96]
    ldp     x14, x15, [sp, #112]
    ldp     x16, x17, [sp, #128]
    ldp     x18, x19, [sp, #144]
    ldp     x20, x21, [sp, #160]
    ldp     x22, x23, [sp, #176]
    ldp     x24, x25, [sp, #192]
    ldp     x26, x27, [sp, #208]
    ldp     x28, x29, [sp, #224]
    ldr     x30,      [sp, #240]

    add     sp, sp, #288
    eret
.endm

/* ============================================================
 * Exception Vector Table
 * Must be aligned to 2048 bytes (0x800)
 * 16 entries total, each exactly 128 bytes (0x80)
 * ============================================================ */
.balign 2048
.global vectors_start
vectors_start:

    /* ── 1. Current EL with SP0 (0x000 - 0x180) ─────────────── */
    .balign 128
    b       entry_el1_sync

    .balign 128
    b       entry_el1_irq

    .balign 128
    b       entry_el1_fiq

    .balign 128
    b       entry_el1_serror

    /* ── 2. Current EL with SPx (0x200 - 0x380) ─────────────── */
    .balign 128
    b       entry_el1_sync

    .balign 128
    b       entry_el1_irq

    .balign 128
    b       entry_el1_fiq

    .balign 128
    b       entry_el1_serror

    /* ── 3. Lower EL using AArch64 (0x400 - 0x580) ──────────── */
    .balign 128
    b       entry_unhandled

    .balign 128
    b       entry_unhandled

    .balign 128
    b       entry_unhandled

    .balign 128
    b       entry_unhandled

    /* ── 4. Lower EL using AArch32 (0x600 - 0x780) ──────────── */
    .balign 128
    b       entry_unhandled

    .balign 128
    b       entry_unhandled

    .balign 128
    b       entry_unhandled

    .balign 128
    b       entry_unhandled

/* ============================================================
 * Exception Handlers Out of Table
 * ============================================================ */

entry_el1_sync:
    SAVE_ALL
    mov     x0, sp
    bl      handle_sync_exception
    RESTORE_ALL

entry_el1_irq:
    SAVE_ALL
    mov     x0, sp
    bl      handle_irq_exception
    RESTORE_ALL

entry_el1_fiq:
    SAVE_ALL
    mov     x0, sp
    bl      handle_fiq_exception
    RESTORE_ALL

entry_el1_serror:
    SAVE_ALL
    mov     x0, sp
    bl      handle_serror_exception
    RESTORE_ALL

entry_unhandled:
    SAVE_ALL
    mov     x0, sp
    bl      handle_unhandled_exception
    RESTORE_ALL

/* ============================================================
 * set_vbar_el1(uint64_t addr)
 * ============================================================ */
.global set_vbar_el1
set_vbar_el1:
    msr     vbar_el1, x0
    isb
    ret

/* ============================================================
 * cpu_switch_to(cpu_context_t* prev, cpu_context_t* next)
 *
 * x0 = prev context pointer
 * x1 = next context pointer
 * ============================================================ */
.global cpu_switch_to
cpu_switch_to:
    cbz     x0, 1f
    stp     x19, x20, [x0, #0]
    stp     x21, x22, [x0, #16]
    stp     x23, x24, [x0, #32]
    stp     x25, x26, [x0, #48]
    stp     x27, x28, [x0, #64]
    stp     x29, x30, [x0, #80]
    mov     x2, sp
    str     x2,       [x0, #96]

1:
    ldp     x19, x20, [x1, #0]
    ldp     x21, x22, [x1, #16]
    ldp     x23, x24, [x1, #32]
    ldp     x25, x26, [x1, #48]
    ldp     x27, x28, [x1, #64]
    ldp     x29, x30, [x1, #80]
    ldr     x2,       [x1, #96]
    mov     sp, x2

    ret
