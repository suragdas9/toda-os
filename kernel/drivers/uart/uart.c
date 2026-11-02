/**
 * Toda OS — UART Driver Implementation (PL011)
 *
 * PL011 is ARM's standard UART controller.
 * QEMU virt machine emulates it at 0x09000000.
 * No external libraries. Pure register writes.
 *
 * Copyright (c) Toda OS Project
 */

#include "uart.h"

/* ================================================
 * PL011 UART Register Map
 * Each register is at UART0_BASE + offset
 * ================================================ */
#define UART_DR             (*((volatile uint32_t*)(UART0_BASE + 0x00))) /* Data Register */
#define UART_FR             (*((volatile uint32_t*)(UART0_BASE + 0x18))) /* Flag Register */
#define UART_IBRD           (*((volatile uint32_t*)(UART0_BASE + 0x24))) /* Integer Baud Rate */
#define UART_FBRD           (*((volatile uint32_t*)(UART0_BASE + 0x28))) /* Fractional Baud Rate */
#define UART_LCR_H          (*((volatile uint32_t*)(UART0_BASE + 0x2C))) /* Line Control */
#define UART_CR             (*((volatile uint32_t*)(UART0_BASE + 0x30))) /* Control Register */
#define UART_IMSC           (*((volatile uint32_t*)(UART0_BASE + 0x38))) /* Interrupt Mask */

/* Flag Register bits */
#define UART_FR_TXFF        BIT(5)   /* TX FIFO Full */
#define UART_FR_RXFE        BIT(4)   /* RX FIFO Empty */
#define UART_FR_BUSY        BIT(3)   /* UART Busy */

/* Control Register bits */
#define UART_CR_UARTEN      BIT(0)   /* UART Enable */
#define UART_CR_TXE         BIT(8)   /* TX Enable */
#define UART_CR_RXE         BIT(9)   /* RX Enable */

/* Line Control bits */
#define UART_LCR_FEN        BIT(4)   /* Enable FIFOs */
#define UART_LCR_WLEN_8     (0x3 << 5) /* 8-bit word length */

/**
 * uart_init — Initialize UART for debug output
 * Baud rate: 115200, 8N1 format
 */
void uart_init(void) {
    /* Step 1: Disable UART */
    UART_CR = 0;

    /* Step 2: Wait for any ongoing transmission to finish */
    while (UART_FR & UART_FR_BUSY);

    /* Step 3: Set baud rate to 115200
     * Formula: IBRD = UARTCLK / (16 * Baud)
     * QEMU clock = 24MHz → IBRD = 24000000 / (16 * 115200) = 13
     * FBRD = fractional part */
    UART_IBRD = 13;
    UART_FBRD = 1;

    /* Step 4: Set 8-bit data, enable FIFOs */
    UART_LCR_H = UART_LCR_WLEN_8 | UART_LCR_FEN;

    /* Step 5: Disable all interrupts */
    UART_IMSC = 0;

    /* Step 6: Enable UART, TX, RX */
    UART_CR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

/**
 * uart_putc — Send one character
 */
void uart_putc(char c) {
    /* Wait until TX FIFO has space */
    while (UART_FR & UART_FR_TXFF);
    UART_DR = (uint32_t)c;
}

/**
 * uart_print — Send a null-terminated string
 */
void uart_print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r');   /* Windows/serial needs CR+LF */
        }
        uart_putc(*str++);
    }
}

/**
 * uart_newline — Print a new line
 */
void uart_newline(void) {
    uart_putc('\r');
    uart_putc('\n');
}

/**
 * uart_print_hex — Print a 64-bit number in hex
 * Example: uart_print_hex(0xDEADBEEF) → "0xDEADBEEF"
 */
void uart_print_hex(uint64_t val) {
    const char hex_chars[] = "0123456789ABCDEF";
    char buf[18];
    int i;

    buf[0]  = '0';
    buf[1]  = 'x';
    buf[17] = '\0';

    for (i = 16; i >= 2; i--) {
        buf[i] = hex_chars[val & 0xF];
        val >>= 4;
    }
    uart_print(buf);
}

/**
 * uart_print_dec — Print a 64-bit number in decimal
 */
void uart_print_dec(uint64_t val) {
    char buf[21];
    int i = 19;

    buf[20] = '\0';

    if (val == 0) {
        uart_putc('0');
        return;
    }

    while (val > 0 && i >= 0) {
        buf[i--] = '0' + (val % 10);
        val /= 10;
    }
    uart_print(&buf[i + 1]);
}

