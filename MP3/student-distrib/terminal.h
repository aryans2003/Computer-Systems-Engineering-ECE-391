#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "keyboard.h"
#include "lib.h"

int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_open (const uint8_t* filename);
int32_t terminal_close (int32_t fd);
int32_t terminal_switch(int terminal);
void set_enter();

#endif /* _TERMINAL_H */
