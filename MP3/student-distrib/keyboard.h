#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

/*Keyboard IRQ index*/
#define keyboard_irq                1
#define keyboard_data_port          0x60

#define char_z                      122
#define char_a                      97

#define L_SCANCODE                  38

#define SCANCODES                   58
#define CAPS_LOCK_PRESS             0x3A
#define LSHIFT_PRESS                0x2A
#define LSHIFT_RELEASE              0xAA
#define RSHIFT_PRESS                0x36
#define RSHIFT_RELEASE              0xB6
#define LRCTRL_PRESS                0x1D
#define LRCTRL_RELEASE              0x9D
#define LRALT_PRESS                 0x38
#define LRALT_RELEASE               0xB8
#define F1_PRESS                    0x3B
#define F1_RELEASE                  0xBB
#define F2_PRESS                    0x3C	
#define F2_RELEASE                  0xBC
#define F3_PRESS                    0x3D
#define F3_RELEASE                  0xBD

#define ENTER                       '\n'
#define BACKSPACE                   0x08

#define BUFFER_LENGTH               128

char keyboard_buffer[BUFFER_LENGTH];
/* Initialize the keyboard */
void init_keyboard();

/* Handles keyboard interrupt */
void keyboard_handler();

uint8_t get_curr_terminal();
int get_kbd_buffer();
void set_kbd_buffer(int kbd_buf);
#endif /* _KEYBOARD_H */
