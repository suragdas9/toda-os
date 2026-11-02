/**
 * Toda OS — Kernel Main Entry Point
 *
 * Boot sequence:
 *  1. UART (Debug Serial)
 *  2. Security Verification (Boot Chain Guard)
 *  3. Memory Manager (Page Frame Allocator & Heap)
 *  4. Interrupt Controller (ARM64 Vector Table & GICv2)
 *  5. System Timer (ARM Generic Timer 1000 Hz)
 *  6. Process Scheduler (Preemptive Round-Robin)
 *  7. Runtime Security
 *  8. Toda Init & Interactive Shell
 *
 * Copyright (c) Toda OS Project
 */

#include "kernel.h"
#include "shell.h"
#include "../drivers/uart/uart.h"
#include "../drivers/timer/timer.h"
#include "../drivers/display/fb.h"
#include "../ui/gui.h"
#include "../mm/memory.h"
#include "../security/security.h"
#include "../proc/process.h"
#include "../arch/arm64/interrupts.h"

void kernel_main(void) {
    /* Phase 1: Initialize UART serial console */
    uart_init();

    uart_print("\n");
    uart_print(ANSI_CYAN "##################################################\n" ANSI_RESET);
    uart_print(ANSI_CYAN "##                                              ##\n" ANSI_RESET);
    uart_print(ANSI_CYAN "##              " ANSI_YELLOW "TODA  OS  v" TODA_OS_VERSION " " ANSI_CYAN "              ##\n" ANSI_RESET);
    uart_print(ANSI_CYAN "##            Codename : " ANSI_YELLOW TODA_OS_CODENAME "                " ANSI_CYAN "##\n" ANSI_RESET);
    uart_print(ANSI_CYAN "##         Architecture : " ANSI_YELLOW TODA_OS_ARCH "                 " ANSI_CYAN "##\n" ANSI_RESET);
    uart_print(ANSI_CYAN "##                                              ##\n" ANSI_RESET);
    uart_print(ANSI_CYAN "##################################################\n" ANSI_RESET);
    uart_print("\nBooting Toda OS from scratch...\n\n");

    /* Phase 2: Security verification */
    uart_print("[BOOT] Phase 2: Security verification...\n");
    if (boot_verify_chain() != VERIFY_OK) {
        kernel_panic("BOOT CHAIN COMPROMISED.");
    }
    uart_print("[BOOT] Security: " ANSI_GREEN "PASSED" ANSI_RESET "\n\n");

    /* Phase 3: Memory manager */
    uart_print("[BOOT] Phase 3: Memory manager...\n");
    memory_init();
    uart_print("[BOOT] Memory: " ANSI_GREEN "READY" ANSI_RESET "\n\n");

    /* Phase 3.5: Mobile Graphical Display (Turn screen ON) */
    uart_print("[BOOT] Phase 3.5: Mobile Graphical UI...\n");
    gui_init();
    uart_print("[BOOT] Display UI: " ANSI_GREEN "READY" ANSI_RESET "\n\n");

    /* Phase 4: Interrupts & ARM GICv2 */
    uart_print("[BOOT] Phase 4: Interrupt controller & vectors...\n");
    interrupts_init();
    uart_print("[BOOT] Interrupts: " ANSI_GREEN "READY" ANSI_RESET "\n\n");

    /* Phase 5: System Timer (1000 Hz / 1ms periodic tick) */
    uart_print("[BOOT] Phase 5: System timer (1000 Hz)...\n");
    timer_init();
    uart_print("[BOOT] Timer: " ANSI_GREEN "RUNNING" ANSI_RESET "\n\n");

    /* Phase 6: Process manager & scheduler */
    uart_print("[BOOT] Phase 6: Process manager & scheduler...\n");
    process_init();
    uart_print("[BOOT] Process manager: " ANSI_GREEN "READY" ANSI_RESET "\n\n");

    /* Phase 7: Runtime security */
    uart_print("[BOOT] Phase 7: Runtime security...\n");
    security_init();
    uart_print("[BOOT] Runtime security: " ANSI_GREEN "ACTIVE" ANSI_RESET "\n\n");

    /* Phase 8: System Init */
    uart_print("[BOOT] Phase 8: Starting Toda Init...\n");
    toda_init();

    kernel_panic("kernel_main: toda_init returned!");
}

void toda_init(void) {
    uart_print("[INIT] Toda Init starting system services...\n");

    /* Unmask hardware interrupts so timer tick begins driving scheduler */
    irq_enable();
    uart_print("[INIT] Hardware interrupts: " ANSI_GREEN "ENABLED" ANSI_RESET "\n");

    uart_print("[INIT] All system services started.\n");
    uart_print("\n");
    uart_print(ANSI_GREEN "##################################################\n" ANSI_RESET);
    uart_print(ANSI_GREEN "##         TODA OS IS RUNNING!                  ##\n" ANSI_RESET);
    uart_print(ANSI_GREEN "##################################################\n" ANSI_RESET);

    /* Initialize and render Toda OS Mobile Smartphone UI */
    gui_init();

    /* Launch interactive shell */
    shell_run();
}

void kernel_halt(void) {
    uart_print("\n[KERNEL] System halted.\n");
    while (1) {
        asm volatile("wfe");
    }
}

void kernel_panic(const char* message) {
    uart_print("\n" ANSI_RED "*** TODA OS KERNEL PANIC ***\nReason: ");
    uart_print(message);
    uart_print(ANSI_RESET "\n");
    kernel_halt();
}
