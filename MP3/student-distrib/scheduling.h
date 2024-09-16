#ifndef _SCHEDULING_H
#define _SCHEDULING_H
#include "lib.h"
#include "i8259.h"
#include "terminal.h"
#include "system_calls.h"

#define PIT_CHANNEL0 0x40
#define PIT_CMD_REG  0x43
#define PIT_irq      0
#define BASE_FREQ    1193182

void init_PIT();
void PIT_handler();
uint8_t get_active_terminal();
#endif /* _SCHEDULING_H */
