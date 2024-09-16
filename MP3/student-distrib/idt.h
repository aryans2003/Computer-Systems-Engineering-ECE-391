#ifndef _idt_H
#define _idt_H

#include "x86_desc.h"
#include "system_calls.h"

#ifndef ASM

#define SYSTEM_CALLS            0x80
#define PIT_CALLS               32
#define KEYBOARD_CALLS          33
#define RTC_CALLS               40
#define NUM_EXCEPTIONS          20
#define RESTRICTED_EXCEPTION    15

void init_idt();

extern void division_error();
extern void debug();
extern void non_maskable_interrupt();
extern void breakpoint();
extern void overflow();
extern void bound_range_exceeded();
extern void invalid_opcode();
extern void device_not_available();
extern void double_fault();
extern void coprocessor_segment_overrun();
extern void invalid_tss();
extern void segment_not_present();
extern void stack_segment_fault();
extern void general_protection_fault();
extern void page_fault();
extern void asm_floating_point_exception();
extern void alignment_check();
extern void machine_check();
extern void simd_floating_point_exception();
extern void system_call();
extern void keyboard_call();
extern void rtc_call();
extern void pit_call();

#endif /* ASM */
#endif /* _idt_H */
