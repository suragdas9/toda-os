/**
 * Toda OS — Memory Manager Implementation
 *
 * Physical Memory Layout (QEMU virt):
 *
 *  0x40000000  ┌─────────────────┐
 *              │   Kernel Code   │  ← Toda OS kernel lives here
 *  0x40100000  ├─────────────────┤
 *              │   Page Bitmap   │  ← Tracks which pages are free/used
 *  0x40200000  ├─────────────────┤
 *              │   Kernel Heap   │  ← kmalloc memory pool
 *  0x40400000  ├─────────────────┤
 *              │                 │
 *              │   Free Pages    │  ← Available for processes
 *              │                 │
 *  0x48000000  └─────────────────┘  ← End of 128MB RAM
 *
 * Copyright (c) Toda OS Project
 */

#include "memory.h"
#include "../drivers/uart/uart.h"

/* ================================================
 * Page Bitmap
 * One byte per page → 0=free, 1=used, 2=reserved
 * For 128MB RAM with 4KB pages = 32768 pages
 * ================================================ */
#define TOTAL_PAGES         (PHYS_MEM_SIZE / PAGE_SIZE)
#define BITMAP_BASE         (PHYS_MEM_BASE + 0x100000ULL)  /* 1MB into RAM */
#define HEAP_BASE           (PHYS_MEM_BASE + 0x200000ULL)  /* 2MB into RAM */
#define HEAP_SIZE           (0x200000ULL)                   /* 2MB heap */
#define FREE_PAGES_BASE     (PHYS_MEM_BASE + 0x400000ULL)  /* 4MB mark */

static uint8_t* page_bitmap = (uint8_t*)BITMAP_BASE;
static uint64_t total_pages = 0;
static uint64_t free_pages  = 0;

/* Heap state */
static uint8_t* heap_ptr    = (uint8_t*)HEAP_BASE;
static uint8_t* heap_end    = (uint8_t*)(HEAP_BASE + HEAP_SIZE);

/**
 * memory_init — Set up the memory manager
 * Called once at boot from kernel_main
 */
void memory_init(void) {
    uint64_t i;
    uint64_t reserved_pages;

    total_pages = TOTAL_PAGES;

    /* Mark all pages as free first */
    toda_memset(page_bitmap, PAGE_FREE, total_pages);

    /* Mark pages below FREE_PAGES_BASE as reserved
     * (kernel code + bitmap + heap live there) */
    reserved_pages = (FREE_PAGES_BASE - PHYS_MEM_BASE) / PAGE_SIZE;
    for (i = 0; i < reserved_pages; i++) {
        page_bitmap[i] = PAGE_RESERVED;
    }

    free_pages = total_pages - reserved_pages;

    uart_print("[MEMORY]   Total RAM : ");
    uart_print_dec(PHYS_MEM_SIZE / (1024 * 1024));
    uart_print(" MB\n");

    uart_print("[MEMORY]   Free pages: ");
    uart_print_dec(free_pages);
    uart_print(" (");
    uart_print_dec((free_pages * PAGE_SIZE) / (1024 * 1024));
    uart_print(" MB)\n");
}

/**
 * page_alloc — Allocate one free 4KB page
 * Returns pointer to page, or NULL if out of memory
 */
void* page_alloc(void) {
    uint64_t i;

    for (i = 0; i < total_pages; i++) {
        if (page_bitmap[i] == PAGE_FREE) {
            page_bitmap[i] = PAGE_USED;
            free_pages--;
            return (void*)(PHYS_MEM_BASE + (i * PAGE_SIZE));
        }
    }
    return NULL;  /* Out of memory! */
}

/**
 * pages_alloc — Allocate n contiguous pages
 */
void* pages_alloc(uint64_t n) {
    uint64_t i, j;
    uint64_t found;

    for (i = 0; i <= total_pages - n; i++) {
        found = 1;
        for (j = 0; j < n; j++) {
            if (page_bitmap[i + j] != PAGE_FREE) {
                found = 0;
                break;
            }
        }
        if (found) {
            for (j = 0; j < n; j++) {
                page_bitmap[i + j] = PAGE_USED;
            }
            free_pages -= n;
            return (void*)(PHYS_MEM_BASE + (i * PAGE_SIZE));
        }
    }
    return NULL;
}

/**
 * page_free — Free a previously allocated page
 */
void page_free(void* addr) {
    uint64_t page_idx;

    if (addr == NULL) return;

    page_idx = ((uint64_t)addr - PHYS_MEM_BASE) / PAGE_SIZE;

    if (page_idx < total_pages && page_bitmap[page_idx] == PAGE_USED) {
        page_bitmap[page_idx] = PAGE_FREE;
        free_pages++;
    }
}

/**
 * kmalloc — Simple bump allocator for kernel heap
 * Fast but cannot free individual allocations
 * Good enough for kernel boot structures
 */
void* kmalloc(size_t size) {
    void* ptr;

    /* Align to 8 bytes */
    size = ALIGN(size, 8);

    if (heap_ptr + size > heap_end) {
        return NULL;  /* Heap exhausted */
    }

    ptr = heap_ptr;
    heap_ptr += size;
    return ptr;
}

/**
 * kfree — Free kernel allocation
 * (Simple allocator — just zero the memory for now)
 */
void kfree(void* ptr) {
    (void)ptr;  /* TODO: implement proper free list */
}

/* ================================================
 * Memory utility functions
 * Our own versions — no libc
 * ================================================ */

void* toda_memset(void* dst, int val, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    while (n--) *d++ = (uint8_t)val;
    return dst;
}

void* toda_memcpy(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;
    while (n--) *d++ = *s++;
    return dst;
}

int toda_memcmp(const void* a, const void* b, size_t n) {
    const uint8_t* pa = (const uint8_t*)a;
    const uint8_t* pb = (const uint8_t*)b;
    while (n--) {
        if (*pa != *pb) return (int)*pa - (int)*pb;
        pa++; pb++;
    }
    return 0;
}

/* Compiler builtin support */
void* memcpy(void* dst, const void* src, size_t n) {
    return toda_memcpy(dst, src, n);
}

void* memset(void* dst, int val, size_t n) {
    return toda_memset(dst, val, n);
}

int memcmp(const void* a, const void* b, size_t n) {
    return toda_memcmp(a, b, n);
}

/* Memory stats */
uint64_t memory_total(void) { return PHYS_MEM_SIZE; }
uint64_t memory_free(void)  { return free_pages * PAGE_SIZE; }
uint64_t memory_used(void)  { return (total_pages - free_pages) * PAGE_SIZE; }

