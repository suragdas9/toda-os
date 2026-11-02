/**
 * Toda OS — Process Manager & Round-Robin Scheduler
 *
 * Implements cooperative and preemptive multitasking for ARM64.
 * Pure custom implementation — NO POSIX, NO Linux.
 *
 * Copyright (c) Toda OS Project
 */

#include "process.h"
#include "../drivers/uart/uart.h"
#include "../mm/memory.h"
#include "../arch/arm64/interrupts.h"

/* Process Table */
static pcb_t process_table[MAX_PROCESSES] __attribute__((aligned(16)));
static int   current_proc_idx = 0;
static uint32_t next_pid      = 1;
static uint32_t active_count  = 0;

/* Helper: string copy without libc */
static void str_copy(char* dst, const char* src, size_t max_len) {
    size_t i = 0;
    while (src && src[i] && i < max_len - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Time slice allotted based on priority level */
static uint32_t get_priority_slice(uint8_t priority) {
    switch (priority) {
        case PRIORITY_REALTIME: return 20;  /* 20ms */
        case PRIORITY_SYSTEM:   return 15;  /* 15ms */
        case PRIORITY_HIGH:     return 10;  /* 10ms */
        case PRIORITY_NORMAL:   return 8;   /* 8ms  */
        case PRIORITY_LOW:      return 5;   /* 5ms  */
        default:                return DEFAULT_TIME_SLICE;
    }
}

/**
 * process_entry_trampoline — Entry wrapper for brand new processes
 *
 * When cpu_switch_to jumps to a new process for the first time,
 * execution lands here. We re-enable interrupts, run the target
 * task function, and automatically invoke process_exit() if it returns.
 */
static void process_entry_trampoline(void) {
    /* Unmask interrupts in the new process context */
    irq_enable();

    pcb_t* cur = process_get_current();
    if (cur && cur->context.x19) {
        task_fn_t task = (task_fn_t)cur->context.x19;
        task();
    }

    process_exit();
}

/**
 * Background demo task to showcase multitasking
 */
static void bg_system_monitor(void) {
    while (1) {
        /* Monitor task yields to give shell CPU priority */
        process_yield();
    }
}

/**
 * Background idle task
 */
static void bg_idle_worker(void) {
    while (1) {
        asm volatile("wfe");
        process_yield();
    }
}

/**
 * process_init — Initialize the process subsystem
 * Sets up Process 0 (Kernel Shell / Init) and background tasks.
 */
void process_init(void) {
    toda_memset(process_table, 0, sizeof(process_table));

    for (uint32_t i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_STATE_UNUSED;
    }

    /* Process 0: Current kernel shell thread running on boot stack */
    pcb_t* p0 = &process_table[0];
    p0->pid = next_pid++;
    str_copy(p0->name, "kernel_shell", PROCESS_NAME_MAX);
    p0->state = PROCESS_STATE_RUNNING;
    p0->priority = PRIORITY_SYSTEM;
    p0->time_slice = get_priority_slice(PRIORITY_SYSTEM);
    p0->total_ticks = 0;
    p0->stack_base = (void*)KERNEL_LOAD_ADDR; /* Uses bootloader stack */
    p0->stack_size = KERNEL_STACK_SIZE;

    current_proc_idx = 0;
    active_count = 1;

    /* Create background helper tasks */
    process_create(bg_system_monitor, "sys_monitor", PRIORITY_NORMAL);
    process_create(bg_idle_worker, "idle_worker", PRIORITY_LOW);

    uart_print("[PROCESS]  Process table initialized (max ");
    uart_print_dec(MAX_PROCESSES);
    uart_print(" processes)\n");
}

/**
 * process_create — Allocate and start a new process
 */
int32_t process_create(task_fn_t fn, const char* name, uint8_t priority) {
    if (fn == NULL) return TODA_ERROR;

    /* Find an empty slot in the process table */
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_STATE_UNUSED) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        uart_print("[PROCESS] ERROR: Process table full!\n");
        return TODA_NOMEM;
    }

    /* Allocate one 4KB page for the process stack */
    void* stack = page_alloc();
    if (stack == NULL) {
        uart_print("[PROCESS] ERROR: Out of memory for process stack!\n");
        return TODA_NOMEM;
    }

    /* Clear the allocated stack */
    toda_memset(stack, 0, PAGE_SIZE);

    pcb_t* p = &process_table[slot];
    p->pid = next_pid++;
    str_copy(p->name, name ? name : "unnamed", PROCESS_NAME_MAX);

    if (priority >= PRIORITY_LEVELS) {
        priority = PRIORITY_LOW;
    }
    p->priority = priority;
    p->time_slice = get_priority_slice(priority);
    p->total_ticks = 0;
    p->stack_base = stack;
    p->stack_size = PAGE_SIZE;

    /* Stack grows downwards: align stack top to 16 bytes */
    uint64_t stack_top = (((uint64_t)stack) + PAGE_SIZE) & ~0xFULL;

    /* Initialize register context */
    toda_memset(&p->context, 0, sizeof(cpu_context_t));
    p->context.sp  = stack_top;
    p->context.lr  = (uint64_t)process_entry_trampoline;
    p->context.fp  = 0;
    p->context.x19 = (uint64_t)fn;  /* Store target function in callee-saved register x19 */

    p->state = PROCESS_STATE_READY;
    active_count++;

    return (int32_t)p->pid;
}

/**
 * process_yield — Relinquish CPU to the next ready process
 * Implements Round-Robin scheduling across priority levels.
 */
void process_yield(void) {
    /* Look for the next READY process */
    int next_idx = -1;

    /* Search for highest priority ready process starting from current index */
    for (uint8_t prio = 0; prio < PRIORITY_LEVELS; prio++) {
        for (int i = 1; i <= MAX_PROCESSES; i++) {
            int idx = (current_proc_idx + i) % MAX_PROCESSES;
            if (process_table[idx].state == PROCESS_STATE_READY &&
                process_table[idx].priority == prio) {
                next_idx = idx;
                break;
            }
        }
        if (next_idx != -1) break;
    }

    /* If no process of other priorities found, check if current is still running/ready */
    if (next_idx == -1) {
        if (process_table[current_proc_idx].state == PROCESS_STATE_RUNNING ||
            process_table[current_proc_idx].state == PROCESS_STATE_READY) {
            return; /* Continue running current process */
        }
        /* Fallback: any ready process */
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (process_table[i].state == PROCESS_STATE_READY) {
                next_idx = i;
                break;
            }
        }
    }

    if (next_idx == -1 || next_idx == current_proc_idx) {
        return; /* Nothing else to switch to */
    }

    pcb_t* prev = &process_table[current_proc_idx];
    pcb_t* next = &process_table[next_idx];

    if (prev->state == PROCESS_STATE_RUNNING) {
        prev->state = PROCESS_STATE_READY;
    }
    next->state = PROCESS_STATE_RUNNING;
    current_proc_idx = next_idx;

    /* Switch CPU execution context */
    cpu_switch_to(&prev->context, &next->context);
}

void scheduler_yield(void) {
    process_yield();
}

/**
 * process_exit — Terminate the current process
 */
void process_exit(void) {
    pcb_t* cur = process_get_current();
    if (cur == NULL) return;

    /* Never terminate PID 1 (kernel shell) */
    if (cur->pid == 1) {
        while (1) { process_yield(); }
    }

    cur->state = PROCESS_STATE_ZOMBIE;
    if (active_count > 0) active_count--;

    /* Free stack memory immediately */
    if (cur->stack_base) {
        page_free(cur->stack_base);
        cur->stack_base = NULL;
    }

    /* Yield immediately to allow other processes to run */
    process_yield();

    /* Should not be reached */
    while (1) { asm volatile("wfe"); }
}

/**
 * scheduler_tick — Called on every 1ms timer interrupt
 */
void scheduler_tick(void) {
    pcb_t* cur = process_get_current();
    if (!cur) return;

    cur->total_ticks++;

    if (cur->time_slice > 0) {
        cur->time_slice--;
    }

    /* When time slice expires, reset */
    if (cur->time_slice == 0) {
        cur->time_slice = get_priority_slice(cur->priority);
    }
}

/**
 * scheduler_start — Begin multi-tasking
 */
void scheduler_start(void) {
    pcb_t* cur = process_get_current();
    if (cur) {
        cur->state = PROCESS_STATE_RUNNING;
    }
    irq_enable();
}

/**
 * process_reap_zombies — Clean up terminated processes
 */
void process_reap_zombies(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_STATE_ZOMBIE) {
            if (process_table[i].stack_base) {
                page_free(process_table[i].stack_base);
                process_table[i].stack_base = NULL;
            }
            process_table[i].state = PROCESS_STATE_UNUSED;
            process_table[i].pid = 0;
        }
    }
}

/* Inspection APIs */
pcb_t* process_get_current(void) {
    if (current_proc_idx >= 0 && current_proc_idx < MAX_PROCESSES) {
        return &process_table[current_proc_idx];
    }
    return NULL;
}

pcb_t* process_get_by_index(uint32_t index) {
    if (index < MAX_PROCESSES) {
        return &process_table[index];
    }
    return NULL;
}

uint32_t process_get_count(void) {
    return active_count;
}

const char* process_state_name(process_state_t state) {
    switch (state) {
        case PROCESS_STATE_UNUSED:  return "UNUSED";
        case PROCESS_STATE_READY:   return "READY";
        case PROCESS_STATE_RUNNING: return "RUNNING";
        case PROCESS_STATE_BLOCKED: return "BLOCKED";
        case PROCESS_STATE_ZOMBIE:  return "ZOMBIE";
        default:                    return "UNKNOWN";
    }
}
