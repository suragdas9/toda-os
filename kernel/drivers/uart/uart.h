/**
 * Toda OS — UART Driver (PL011)
 * 
 * UART = Universal Asynchronous Receiver/Transmitter
 * This is how we print debug messages during boot.
 * Works on QEMU virt machine (PL011 UART).
 * Will work on real ARM64 phone hardware too.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_UART_H
#define TODA_UART_H

#include "../../core/kernel.h"

void uart_init(void);
void uart_putc(char c);
void uart_print(const char* str);
void uart_print_hex(uint64_t val);
void uart_print_dec(uint64_t val);
void uart_newline(void);

#endif /* TODA_UART_H */

