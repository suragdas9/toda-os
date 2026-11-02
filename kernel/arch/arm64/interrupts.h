/**
 * Toda OS — ARM64 Interrupt & Exception Handling
 *
 * Exception table, GICv2 driver, and IRQ dispatcher for QEMU virt.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_INTERRUPTS_H
#define TODA_INTERRUPTS_H

#include "../../core/kernel.h"

/* GICv2 Base Addresses on QEMU ARM64 virt machine */
#define GICD_BASE               0x08000000ULL   /* Distributor */
#define GICC_BASE               0x08010000ULL   /* CPU Interface */

/* Standard ARM64 Timer PPI Interrupt Numbers */
#define IRQ_VIRTUAL_TIMER       27              /* Virtual timer PPI 11 */
#define IRQ_PHYSICAL_TIMER      30              /* Physical timer PPI 14 */

/* Trap frame saved on stack during exceptions (288 bytes, 16-byte aligned) */
typedef struct trap_frame {
    uint64_t x0,  x1;
    uint64_t x2,  x3;
    uint64_t x4,  x5;
    uint64_t x6,  x7;
    uint64_t x8,  x9;
    uint64_t x10, x11;
    uint64_t x12, x13;
    uint64_t x14, x15;
    uint64_t x16, x17;
    uint64_t x18, x19;
    uint64_t x20, x21;
    uint64_t x22, x23;
    uint64_t x24, x25;
    uint64_t x26, x27;
    uint64_t x28, x29;
    uint64_t x30;
    uint64_t elr;       /* Exception Link Register (faulting PC) */
    uint64_t spsr;      /* Saved Processor State Register */
    uint64_t esr;       /* Exception Syndrome Register */
    uint64_t far;       /* Fault Address Register */
} trap_frame_t;

typedef void (*irq_handler_t)(trap_frame_t* frame);

/* Core API */
void     interrupts_init(void);
void     irq_enable(void);
void     irq_disable(void);
bool     irq_is_enabled(void);

/* GIC Controller API */
void     gic_init(void);
void     gic_enable_irq(uint32_t irq);
void     gic_disable_irq(uint32_t irq);
void     register_irq_handler(uint32_t irq, irq_handler_t handler);

/* C Exception Entry Points (called from vectors.S) */
void     handle_sync_exception(trap_frame_t* frame);
void     handle_irq_exception(trap_frame_t* frame);
void     handle_fiq_exception(trap_frame_t* frame);
void     handle_serror_exception(trap_frame_t* frame);
void     handle_unhandled_exception(trap_frame_t* frame);

/* Assembly vector table address */
extern void* vectors_start;
extern void  set_vbar_el1(uint64_t addr);

#endif /* TODA_INTERRUPTS_H */
