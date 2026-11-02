/**
 * Toda OS — ARM Generic System Timer Driver Header
 *
 * Configures the ARM64 Architectural Generic Timer for periodic 1ms ticks (1000 Hz)
 * to drive the Round-Robin process scheduler.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_TIMER_H
#define TODA_TIMER_H

#include "../../core/kernel.h"

/* QEMU virt machine timer MMIO base address (PL031 RTC / Timer subsystem) */
#define TIMER_BASE          0x09010000ULL

/* Operating frequency: 1000 ticks per second (1ms resolution) */
#define TIMER_HERTZ         1000ULL

/* Core Timer API */
void     timer_init(void);
void     timer_handler(void);
uint64_t timer_get_ticks(void);
uint64_t timer_get_uptime_ms(void);
uint64_t timer_get_frequency(void);
void     timer_sleep_ms(uint64_t ms);

#endif /* TODA_TIMER_H */
