
#include <stdint.h>

/* memory reference types */
typedef uint32_t            x86_realptr_t;
typedef uint32_t            x86_farptr16_t;
typedef uint32_t            x86_offset_t;
typedef uint16_t            x86_segval_t;

static inline x86_realptr_t x86_realptr(const uint16_t s,const uint16_t o) {
    return ((x86_realptr_t)s << ((x86_realptr_t)16U)) + ((x86_realptr_t)o);
}

static inline uint16_t x86_realseg(const x86_realptr_t p) {
    return (uint16_t)(p >> (x86_realptr_t)16U);
}

static inline uint16_t x86_realoff(const x86_realptr_t p) {
    return (uint16_t)(p & (x86_realptr_t)0xFFFFU);
}

static inline x86_farptr16_t x86_farptr16(const uint16_t s,const uint16_t o) {
    return ((x86_farptr16_t)s << ((x86_farptr16_t)16U)) + ((x86_farptr16_t)o);
}

static inline uint16_t x86_far16seg(const x86_farptr16_t p) {
    return (uint16_t)(p >> (x86_farptr16_t)16U);
}

static inline uint16_t x86_far16off(const x86_farptr16_t p) {
    return (uint16_t)(p & (x86_farptr16_t)0xFFFFU);
}

/* virtual memory address (after segment base+offset, before translation by page tables) */
typedef uint32_t            x86_virtaddr_t;

/* physical memory address (after virt->phys page mapping). this is what gets sent out to the bus from the CPU. */
typedef uint32_t            x86_physaddr_t;

