#define ASM     1
#define SYSTEM_CALLS    0x80
#define PIT_CALLS    32
#define KEYBOARD_CALLS    33
#define RTC_CALLS    40

#define EXCEPTION(function_name, vector)            \
    .globl function_name                            ;\
    function_name:                                  ;\
        pushal                                      ;\
        pushfl                                      ;\
        pushl $vector                               ;\
        call exc_handler                            ;\
        addl $4, %esp                               ;\
        popfl                                       ;\
        popal                                       ;\
        iret

#define KEYBOARD_INTERRUPT(function_name, vector) \
    .globl function_name                            ;\
    function_name:                                  ;\
    call keyboard_handler                           ;\
        iret

#define RTC_INTERRUPT(function_name, vector) \
    .globl function_name                            ;\
    function_name:                                  ;\
    call RTC_handler                                ;\
        iret

#define PIT_INTERRUPT(function_name, vector) \
    .globl function_name                            ;\
    function_name:                                  ;\
    call PIT_handler                                ;\
        iret

EXCEPTION(division_error, 0);
EXCEPTION(debug, 1);
EXCEPTION(non_maskable_interrupt, 2);
EXCEPTION(breakpoint, 3);
EXCEPTION(overflow, 4);
EXCEPTION(bound_range_exceeded, 5);
EXCEPTION(invalid_opcode, 6);
EXCEPTION(device_not_available, 7);
EXCEPTION(double_fault, 8);
EXCEPTION(coprocessor_segment_overrun, 9);
EXCEPTION(invalid_tss, 10);
EXCEPTION(segment_not_present, 11);
EXCEPTION(stack_segment_fault, 12);
EXCEPTION(general_protection_fault, 13);
EXCEPTION(page_fault, 14);
EXCEPTION(asm_floating_point_exception, 16);
EXCEPTION(alignment_check, 17);
EXCEPTION(machine_check, 18);
EXCEPTION(simd_floating_point_exception, 19);
EXCEPTION(system_call, SYSTEM_CALLS);
KEYBOARD_INTERRUPT(keyboard_call, KEYBOARD_CALLS);
RTC_INTERRUPT(rtc_call, RTC_CALLS);
PIT_INTERRUPT(pit_call, PIT_CALLS);
