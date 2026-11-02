/**
 * Toda OS — Process Manager & Scheduler Header
 *
 * Fully from-scratch Round-Robin Process Scheduler
 * No Linux, no POSIX, pure custom kernel design.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_PROCESS_H
#define TODA_PROCESS_H

#include "../core/kernel.h"

/* Maximum concurrent processes supported */
#define MAX_PROCESSES       16
#define PROCESS_NAME_MAX    32
#define PROCESS_STACK_SIZE  4096   /* 4KB per process stack */

/* Default time slice in timer ticks (e.g. 10ms at 1000Hz) */
#define DEFAULT_TIME_SLICE  10

/* Process Lifecycle States */
typedef enum {
    PROCESS_STATE_UNUSED = 0,
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_ZOMBIE
} process_state_t;

/* ARM64 Callee-saved Register Context for Context Switching */
typedef struct __attribute__((aligned(16))) cpu_context {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;   /* x29: Frame Pointer */
    uint64_t lr;   /* x30: Link Register (return / entry address) */
    uint64_t sp;   /* Stack Pointer */
} cpu_context_t;

/* Process Control Block (PCB) */
typedef struct __attribute__((aligned(16))) pcb {
    uint32_t        pid;                    /* Process ID */
    char            name[PROCESS_NAME_MAX]; /* Process name string */
    process_state_t state;                  /* Current execution state */
    uint8_t         priority;               /* Priority level: 0 (Realtime) to 4 (Low) */
    uint32_t        time_slice;             /* Remaining time slice ticks */
    uint32_t        total_ticks;            /* Cumulative execution ticks */
    cpu_context_t   context;                /* Saved CPU registers */
    void*           stack_base;             /* Base address of allocated stack page */
    uint64_t        stack_size;             /* Stack size in bytes */
} pcb_t;

/* Assembly Context Switch Routine */
extern void cpu_switch_to(cpu_context_t* prev, cpu_context_t* next);

/* Process Management API */
void     process_init(void);
int32_t  process_create(task_fn_t fn, const char* name, uint8_t priority);
void     process_exit(void);
void     process_yield(void);
void     scheduler_yield(void);
void     scheduler_tick(void);
void     scheduler_start(void);
void     process_reap_zombies(void);

/* Process Inspection API */
pcb_t*   process_get_current(void);
pcb_t*   process_get_by_index(uint32_t index);
uint32_t process_get_count(void);
const char* process_state_name(process_state_t state);

#endif /* TODA_PROCESS_H */
