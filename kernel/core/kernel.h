/**
 * Toda OS — Core Kernel Types & Definitions
 * 
 * No standard library. No Linux. No borrowed code.
 * Everything defined from scratch.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_KERNEL_H
#define TODA_KERNEL_H

/* ================================================
 * SECTION 1: Basic Types
 * We define our own — no <stdint.h>, no libc
 * ================================================ */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

typedef uint64_t            size_t;
typedef uint64_t            uintptr_t;
typedef int64_t             intptr_t;

typedef _Bool               bool;
#define true                1
#define false               0
#define NULL                ((void*)0)

/* ================================================
 * SECTION 2: Result / Status Codes
 * ================================================ */
typedef int32_t             result_t;

#define TODA_OK             0
#define TODA_ERROR          -1
#define TODA_NOMEM          -2
#define TODA_BUSY           -3
#define TODA_TIMEOUT        -4
#define TODA_DENIED         -5    /* Security rejection */
#define TODA_TAMPERED       -6    /* Boot verification failed */

/* ================================================
 * SECTION 3: OS Identity
 * ================================================ */
#define TODA_OS_NAME        "Toda OS"
#define TODA_OS_VERSION     "0.1.0"
#define TODA_OS_CODENAME    "Genesis"
#define TODA_OS_ARCH        "ARM64"

/* ================================================
 * SECTION 4: Memory Map (QEMU virt machine)
 * Will be updated for real AI Plus hardware later
 * ================================================ */
#define PHYS_MEM_BASE       0x40000000ULL   /* RAM starts here in QEMU */
#define PHYS_MEM_SIZE       0x08000000ULL   /* 128MB for QEMU testing */
#define KERNEL_LOAD_ADDR    0x40080000ULL   /* Where kernel loads */
#define KERNEL_STACK_SIZE   0x00004000ULL   /* 16KB kernel stack */

/* MMIO (Memory Mapped I/O) base for QEMU virt */
#define UART0_BASE          0x09000000ULL   /* UART for debug output */

/* ================================================
 * SECTION 5: Process Priorities
 * ================================================ */
#define PRIORITY_REALTIME   0   /* Highest — touch input, display vsync */
#define PRIORITY_SYSTEM     1   /* System daemons */
#define PRIORITY_HIGH       2   /* Foreground app */
#define PRIORITY_NORMAL     3   /* Background apps */
#define PRIORITY_LOW        4   /* Lowest — idle tasks */
#define PRIORITY_LEVELS     5

/* ================================================
 * SECTION 6: Page Size & Alignment
 * ================================================ */
#define PAGE_SIZE           4096ULL         /* 4KB pages */
#define PAGE_SHIFT          12
#define PAGE_ALIGN(addr)    (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_OF(addr)       ((addr) >> PAGE_SHIFT)

/* ================================================
 * SECTION 7: Useful Macros
 * ================================================ */
#define ARRAY_SIZE(arr)     (sizeof(arr) / sizeof((arr)[0]))
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define ALIGN(val, align)   (((val) + (align) - 1) & ~((align) - 1))
#define BIT(n)              (1ULL << (n))

/* ================================================
 * SECTION 8: Kernel Function Declarations
 * ================================================ */

/* Entry points */
void kernel_main(void);
void toda_init(void);

/* Panic & halt */
void kernel_halt(void);
void kernel_panic(const char* message);

/* Scheduler */
void scheduler_start(void);
void scheduler_yield(void);

/* Process */
typedef void (*task_fn_t)(void);
int32_t process_create(task_fn_t fn, const char* name, uint8_t priority);
void    process_reap_zombies(void);

#endif /* TODA_KERNEL_H */

