#ifndef _exception_handler_H
#define _exception_handler_H

#ifndef ASM

#include "lib.h"
#include "system_calls.h"

#define SYSTEM_CALLS    0x80
#define NUM_EXCEPTIONS  20

struct registers {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t esp;
    uint32_t ebp;
};

char* os_exceptions[NUM_EXCEPTIONS] = {
    "Division Error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved"
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception"
};

void exc_handler(uint32_t id, struct registers regs, uint32_t error_code);

#endif
#endif /* _exception_handler_H */
