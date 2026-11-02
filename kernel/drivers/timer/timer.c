/**
 * Toda OS — System Timer Implementation
 *
 * Utilizes ARM Generic Timer (Virtual Timer) configured for 1000 Hz (1ms tick)
 * QEMU virt machine MMIO base address: 0x09010000
 *
 * Copyright (c) Toda OS Project
 */

#include "timer.h"
#include "../../arch/arm64/interrupts.h"
#include "../../proc/process.h"
#include "../uart/uart.h"

/* Volatile tick counter incremented on every hardware interrupt */
static volatile uint64_t system_ticks       = 0;
static uint64_t          timer_freq         = 0;
static uint64_t          ticks_per_interval = 0;

/* QEMU virt machine PL031 RTC / Timer registers at 0x09010000 */
#define RTC_DR  (*((volatile uint32_t*)(TIMER_BASE + 0x00))) /* Data Register (seconds) */

/**
 * timer_init — Initialize ARM generic timer for 1000 Hz ticks
 */
void timer_init(void) {
    /* 1. Read system counter frequency */
    asm volatile("mrs %0, cntfrq_el0" : "=r"(timer_freq));
    if (timer_freq == 0) {
        timer_freq = 62500000ULL; /* Standard 62.5MHz on QEMU Cortex-A72 */
    }

    /* 2. Compute interval for 1000 Hz (1ms) */
    ticks_per_interval = timer_freq / TIMER_HERTZ;

    /* 3. Program initial countdown into Virtual Timer */
    asm volatile("msr cntv_tval_el0, %0" : : "r"(ticks_per_interval));

    /* 4. Enable Virtual Timer: bit 0 = 1 (ENABLE), bit 1 = 0 (IMASK disabled) */
    asm volatile("msr cntv_ctl_el0, %0" : : "r"(1ULL));

    /* 5. Enable Virtual Timer IRQ (27) and Physical Timer IRQ (30) in GIC */
    gic_enable_irq(IRQ_VIRTUAL_TIMER);
    gic_enable_irq(IRQ_PHYSICAL_TIMER);

    uart_print("[TIMER]    ARM Generic Timer running at ");
    uart_print_dec(timer_freq / 1000000ULL);
    uart_print(" MHz (1000 Hz / 1ms tick)\n");
    uart_print("[TIMER]    MMIO Base: 0x09010000 (RTC seconds: ");
    uart_print_dec((uint64_t)RTC_DR);
    uart_print(")\n");
}

/**
 * timer_handler — Invoked on every timer tick IRQ
 */
void timer_handler(void) {
    system_ticks++;

    /* Reload Virtual Timer for the next 1ms interval */
    asm volatile("msr cntv_tval_el0, %0" : : "r"(ticks_per_interval));

    /* Drive process scheduler */
    scheduler_tick();
}

/**
 * timer_get_ticks — Returns total ticks since boot
 */
uint64_t timer_get_ticks(void) {
    return system_ticks;
}

/**
 * timer_get_uptime_ms — Returns uptime in milliseconds
 */
uint64_t timer_get_uptime_ms(void) {
    return (system_ticks * 1000ULL) / TIMER_HERTZ;
}

/**
 * timer_get_frequency — Returns hardware counter frequency in Hz
 */
uint64_t timer_get_frequency(void) {
    return timer_freq;
}

/**
 * timer_sleep_ms — Delay for specified number of milliseconds
 */
void timer_sleep_ms(uint64_t ms) {
    uint64_t start = timer_get_ticks();
    uint64_t target = (ms * TIMER_HERTZ) / 1000ULL;
    while ((timer_get_ticks() - start) < target) {
        process_yield();
    }
}
