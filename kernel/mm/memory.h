/**
 * Toda OS — Memory Manager
 *
 * Manages all physical RAM in the phone.
 * Uses a simple page frame allocator.
 * Pages = 4KB chunks of memory.
 *
 * Copyright (c) Toda OS Project
 */

#ifndef TODA_MEMORY_H
#define TODA_MEMORY_H

#include "../../core/kernel.h"

/* Page frame states */
#define PAGE_FREE       0
#define PAGE_USED       1
#define PAGE_RESERVED   2

/* Memory init & info */
void     memory_init(void);
uint64_t memory_total(void);
uint64_t memory_free(void);
uint64_t memory_used(void);

/* Physical page allocator */
void*    page_alloc(void);           /* Allocate one 4KB page */
void*    pages_alloc(uint64_t n);    /* Allocate n contiguous pages */
void     page_free(void* addr);      /* Free one page */
void     pages_free(void* addr, uint64_t n);

/* Simple heap allocator (kmalloc/kfree) */
void*    kmalloc(size_t size);       /* Allocate bytes from kernel heap */
void     kfree(void* ptr);           /* Free kernel heap allocation */

/* Memory operations (our own memset/memcpy) */
void*    toda_memset(void* dst, int val, size_t n);
void*    toda_memcpy(void* dst, const void* src, size_t n);
int      toda_memcmp(const void* a, const void* b, size_t n);

#endif /* TODA_MEMORY_H */

