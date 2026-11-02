/**
 * Toda OS — Linker Script
 * Tells the compiler exactly where to put each section in memory
 *
 * Memory layout for QEMU ARM64 virt machine:
 *  0x40080000 → Kernel loads here (TEXT section)
 *
 * Copyright (c) Toda OS Project
 */

ENTRY(_start)

SECTIONS {

    /* Kernel loads at 0x40080000 in RAM */
    . = 0x40080000;

    /* ── .text ─────────────────────────────────────
     * Executable code
     * boot.S must come FIRST (it's our entry point)
     * ─────────────────────────────────────────────*/
    .text : {
        KEEP(*(.text.boot))    /* bootloader entry — must be first! */
        *(.text)
        *(.text.*)
    }

    /* ── .rodata ────────────────────────────────────
     * Read-only data (strings, constants)
     * ─────────────────────────────────────────────*/
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Align to 4KB page boundary */
    . = ALIGN(4096);

    /* ── .data ──────────────────────────────────────
     * Initialized global variables
     * ─────────────────────────────────────────────*/
    .data : {
        *(.data)
        *(.data.*)
    }

    /* ── .bss ───────────────────────────────────────
     * Uninitialized global variables
     * Boot.S zeros this section before kernel_main
     * ─────────────────────────────────────────────*/
    . = ALIGN(8);
    __bss_start = .;
    .bss : {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    }
    . = ALIGN(8);
    __bss_end = .;
}

