/**
 * Toda OS — ARM64 Interrupt & Exception Handling Implementation
 *
 * Configures the ARM Generic Interrupt Controller (GICv2) for QEMU virt
 * and dispatches IRQs, timers, and processor exceptions.
 *
 * Copyright (c) Toda OS Project
 */

#include "interrupts.h"
#include "../../drivers/uart/uart.h"
#include "../../drivers/timer/timer.h"

/* GICv2 Register Access Macros */
#define GICD_REG(offset)        (*(volatile uint32_t*)(GICD_BASE + (offset)))
#define GICC_REG(offset)        (*(volatile uint32_t*)(GICC_BASE + (offset)))

/* Distributor Registers */
#define GICD_CTLR               GICD_REG(0x000)
#define GICD_TYPER              GICD_REG(0x004)
#define GICD_ISENABLER(n)       GICD_REG(0x100 + 4 * (n))
#define GICD_ICENABLER(n)       GICD_REG(0x180 + 4 * (n))
#define GICD_IPRIORITYR(n)      GICD_REG(0x400 + 4 * (n))
#define GICD_ITARGETSR(n)       GICD_REG(0x800 + 4 * (n))
#define GICD_ICFGR(n)           GICD_REG(0xC00 + 4 * (n))

/* CPU Interface Registers */
#define GICC_CTLR               GICC_REG(0x000)
#define GICC_PMR                GICC_REG(0x004)
#define GICC_BPR                GICC_REG(0x008)
#define GICC_IAR                GICC_REG(0x00C)
#define GICC_EOIR               GICC_REG(0x010)

#define MAX_IRQS                64
static irq_handler_t irq_handlers[MAX_IRQS];

/**
 * gic_init — Initialize the Generic Interrupt Controller (v2)
 */
void gic_init(void) {
    /* 1. Disable distributor during configuration */
    GICD_CTLR = 0;

    /* 2. Configure interrupts (IRQs 0-63) */
    for (int i = 0; i < 2; i++) {
        GICD_ICENABLER(i) = 0xFFFFFFFF; /* Disable all initially */
    }

    /* Set default priority (0xA0) for IRQs 0-63 */
    for (int i = 0; i < 16; i++) {
        GICD_IPRIORITYR(i) = 0xA0A0A0A0;
    }

    /* Target CPU 0 (0x01) for SPIs (IRQs 32-63) */
    for (int i = 8; i < 16; i++) {
        GICD_ITARGETSR(i) = 0x01010101;
    }

    /* 3. Enable Distributor */
    GICD_CTLR = 1;

    /* 4. Configure CPU Interface: set Priority Mask to allow all */
    GICC_PMR = 0xFF;

    /* 5. Enable CPU Interface */
    GICC_CTLR = 1;

    /* Clear handler table */
    for (int i = 0; i < MAX_IRQS; i++) {
        irq_handlers[i] = NULL;
    }

    uart_print("[GIC]      GICv2 initialized at 0x08000000 / 0x08010000\n");
}

/**
 * gic_enable_irq — Enable an IRQ line in GIC
 */
void gic_enable_irq(uint32_t irq) {
    if (irq < MAX_IRQS) {
        GICD_ISENABLER(irq / 32) = (1U << (irq % 32));
    }
}

/**
 * gic_disable_irq — Disable an IRQ line in GIC
 */
void gic_disable_irq(uint32_t irq) {
    if (irq < MAX_IRQS) {
        GICD_ICENABLER(irq / 32) = (1U << (irq % 32));
    }
}

/**
 * register_irq_handler — Attach custom handler for an IRQ
 */
void register_irq_handler(uint32_t irq, irq_handler_t handler) {
    if (irq < MAX_IRQS) {
        irq_handlers[irq] = handler;
        gic_enable_irq(irq);
    }
}

/**
 * interrupts_init — Install ARM64 vector table and init GIC
 */
void interrupts_init(void) {
    /* Set Vector Base Address Register for EL1 */
    set_vbar_el1((uint64_t)&vectors_start);

    /* Initialize GICv2 */
    gic_init();

    uart_print("[ARCH]     ARM64 Exception Vector Table installed at VBAR_EL1\n");
}

/**
 * irq_enable — Unmask IRQ interrupts (clear I bit in DAIF)
 */
void irq_enable(void) {
    asm volatile("msr daifclr, #2" ::: "memory");
}

/**
 * irq_disable — Mask IRQ interrupts (set I bit in DAIF)
 */
void irq_disable(void) {
    asm volatile("msr daifset, #2" ::: "memory");
}

/**
 * irq_is_enabled — Test whether IRQs are enabled
 */
bool irq_is_enabled(void) {
    uint64_t daif;
    asm volatile("mrs %0, daif" : "=r"(daif));
    return ((daif & (1ULL << 7)) == 0);
}

/**
 * handle_irq_exception — Dispatches active hardware interrupt
 */
void handle_irq_exception(trap_frame_t* frame) {
    uint32_t iar = GICC_IAR;
    uint32_t irq = iar & 0x3FF;

    /* Spurious interrupt check (1020 - 1023) */
    if (irq >= 1020) {
        return;
    }

    /* Check if it is a timer interrupt */
    if (irq == IRQ_VIRTUAL_TIMER || irq == IRQ_PHYSICAL_TIMER) {
        timer_handler();
    } else if (irq < MAX_IRQS && irq_handlers[irq] != NULL) {
        irq_handlers[irq](frame);
    }

    /* End Of Interrupt */
    GICC_EOIR = iar;
}

/**
 * handle_sync_exception — Handles synchronous faults (data abort, undef, etc.)
 */
void handle_sync_exception(trap_frame_t* frame) {
    uart_print("\n*** SYNCHRONOUS EXCEPTION ***\n");
    uart_print("  Faulting PC (ELR) : ");
    uart_print_hex(frame->elr);
    uart_newline();
    uart_print("  ESR_EL1           : ");
    uart_print_hex(frame->esr);
    uart_newline();
    uart_print("  Fault Addr (FAR)  : ");
    uart_print_hex(frame->far);
    uart_newline();
    uart_print("  SPSR_EL1          : ");
    uart_print_hex(frame->spsr);
    uart_newline();

    kernel_panic("Synchronous kernel exception occurred.");
}

/**
 * handle_fiq_exception — Fast interrupt handler
 */
void handle_fiq_exception(trap_frame_t* frame) {
    (void)frame;
    uart_print("\n[ARCH] Fast Interrupt (FIQ) received.\n");
}

/**
 * handle_serror_exception — System Error handler
 */
void handle_serror_exception(trap_frame_t* frame) {
    uart_print("\n*** SERROR EXCEPTION ***\n");
    uart_print("  Faulting PC (ELR) : ");
    uart_print_hex(frame->elr);
    uart_newline();
    uart_print("  ESR_EL1           : ");
    uart_print_hex(frame->esr);
    uart_newline();
    kernel_panic("Asynchronous SError exception.");
}

/**
 * handle_unhandled_exception — Catch-all for unexpected vector traps
 */
void handle_unhandled_exception(trap_frame_t* frame) {
    uart_print("\n*** UNHANDLED EXCEPTION ***\n");
    uart_print("  ELR : ");
    uart_print_hex(frame->elr);
    uart_newline();
    kernel_panic("Unhandled exception vector.");
}
