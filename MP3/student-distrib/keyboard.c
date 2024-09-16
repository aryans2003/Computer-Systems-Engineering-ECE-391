#include "keyboard.h"
#include "terminal.h"
char ascii_1[SCANCODES] = { 0x0, 0x0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', BACKSPACE, '\t', 'q', 'w',
                   'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', ENTER, 0x0,'a', 's', 'd', 'f', 'g', 'h', 'j', 'k',
                   'l', ';', '\'', '`', 0x0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0x0, 0x0, 0x0, ' '};
char ascii_2[SCANCODES] = { 0x0, 0x0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', BACKSPACE, '\t', 'Q', 'W',
                   'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', ENTER, 0x0,'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K',
                   'L', ':', '"', '~', 0x0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0x0, 0x0, 0x0, ' '};

char caps_lock = 0;
char lshift = 0;
char rshift = 0;
char ctrl = 0;
char alt = 0;
int kbd_buffer = 0;     /*Keyboard buffer index*/
int overflow_block = 0;
char f1 = 0;
char f2 = 0;
char f3 = 0;
uint8_t curr_terminal = 1;

static void clear_buffer();

/* void init_keyboard()
 *  Functionality: Initializes the keyboard interrupt
 *  Arguments: None
 *  Return: None
 ***********************************************************************************
 *  
 */
void init_keyboard() {
    enable_irq(keyboard_irq);
}

/* void keyboard_handler()
 *  Functionality: Interrupt handler for keyboard events
 *  Arguments: None
 *  Return: None
 ***********************************************************************************
 *  
 */
void keyboard_handler() {
    cli();
    int i;
    uint8_t c = inb(keyboard_data_port);
    if(ascii_1[c] == ENTER) {   /*If enter is pressed we put the \n character on the screen and also sets the kbd_buffer to 0*/
        keyboard_buffer[kbd_buffer] = '\n';
        kbd_buffer = 0;
        putc('\n');
        set_enter();
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(ascii_1[c] == '\t') {
        if(!(alt && ctrl && (lshift || rshift))) {
            i = 0;
            while(i < 3 && kbd_buffer != BUFFER_LENGTH-1) {  //3 represents the number of tab spaces
                keyboard_buffer[kbd_buffer] = ' ';
                kbd_buffer++;
                putc(' ');
                i++;
            } 
            send_eoi(keyboard_irq);
            sti();
            return;
        } else {
            i = 0;
            while(i < 3 && kbd_buffer != BUFFER_LENGTH-1) {  //3 represents the number of tab spaces
                keyboard_buffer[kbd_buffer] = ' ';
                kbd_buffer++;
                putc(' ');
                i++;
            }
            while(i < 3) {
                putc(' ');
                i++;
            }
            send_eoi(keyboard_irq);
            sti();
            return;
        }
    }

    /*This block of code manages what happens when backspace is pressed. We decrement the kbd_buffer and set the 
    previous character to a null pointer if kbd_buffer is greater than 0 else does nothing to the keyboard buffer*/
    if(ascii_1[c] == BACKSPACE) {   
        if(kbd_buffer > 0) {
            kbd_buffer--;
            keyboard_buffer[kbd_buffer] = 0x0;
            putc(ascii_1[c]);
            send_eoi(keyboard_irq);
            sti();
            return;
        } else {
            send_eoi(keyboard_irq);
            sti();
            return;
        }
    }

    /*If alt or null is pressed we do nothing*/
    if(c == 0x0) {
        send_eoi(keyboard_irq);
        sti();
        return;
    }

    /*These if statements are to manage what happens when a special key is pressed*/
    if(c == CAPS_LOCK_PRESS) {
        if(caps_lock == 0) {
            caps_lock = 1;
        } else {
            caps_lock = 0;
        }
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == LSHIFT_PRESS) {
        lshift += 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == LSHIFT_RELEASE) {
        lshift -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == RSHIFT_PRESS) {
        rshift += 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == RSHIFT_RELEASE) {
        rshift -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == LRCTRL_PRESS) {
        ctrl += 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == LRCTRL_RELEASE) {
        ctrl -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == LRALT_PRESS) {
        alt += 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == LRALT_RELEASE) {
        alt -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }

    if(c == F1_PRESS) {
        f1 += 1;
        if(alt == 1 && f1 == 1) {
            curr_terminal = 1;
            terminal_switch(1);
        }
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == F1_RELEASE) {
        f1 -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }

    if(c == F2_PRESS) {
        f2 += 1;
        if(alt == 1 && f2 == 1) {
            curr_terminal = 2;
            terminal_switch(2);
        }
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == F2_RELEASE) {
        f2 -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }

    if(c == F3_PRESS) {
        f3 += 1;
        if(alt == 1 && f3 == 1) {
            curr_terminal = 3;
            terminal_switch(3);
        }
        send_eoi(keyboard_irq);
        sti();
        return;
    }
    if(c == F3_RELEASE) {
        f3 -= 1;
        send_eoi(keyboard_irq);
        sti();
        return;
    }

    /*Special clear screen command*/
    if(ctrl == 1 && c == L_SCANCODE) {
        kbd_buffer = 0;
        clear_buffer();
        clear();
        send_eoi(keyboard_irq);
        sti();
        return;
    }

    /*This code controls what happens when any normal character is pressed. It prevents keyboard buffer overflow and
    adds the entered characters into the keyboard buffer*/
    if(kbd_buffer != BUFFER_LENGTH-1) {
        if(lshift || rshift) {
            if(c < SCANCODES) {
                putc(ascii_2[c]);
                keyboard_buffer[kbd_buffer] = ascii_2[c];
                kbd_buffer++;
                send_eoi(keyboard_irq);
                sti();
                return;
            }
        }
        if(c < SCANCODES) {
            if(caps_lock == 1 && ascii_1[c] <= char_z && ascii_1[c] >= char_a) {
                putc(ascii_2[c]);
                keyboard_buffer[kbd_buffer] = ascii_2[c];
                kbd_buffer++;
                send_eoi(keyboard_irq);
                sti();
                return;
            } else if (caps_lock == 0 && ascii_1[c] <= char_z && ascii_1[c] >= char_a) {
                putc(ascii_1[c]);
                keyboard_buffer[kbd_buffer] = ascii_1[c];
                kbd_buffer++;
                send_eoi(keyboard_irq);
                sti();
                return;
            } else {
                putc(ascii_1[c]);
                keyboard_buffer[kbd_buffer] = ascii_1[c];
                kbd_buffer++;
                send_eoi(keyboard_irq);
                sti();
                return;
            }
        }
    } else if(alt && ctrl && (lshift || rshift)) {
        if(lshift || rshift) {
            if(c < SCANCODES) {
                putc(ascii_2[c]);
                send_eoi(keyboard_irq);
                sti();
                return;
            }
        }
        if(c < SCANCODES) {
            if(caps_lock == 1 && ascii_1[c] <= char_z && ascii_1[c] >= char_a) {
                putc(ascii_2[c]);
                send_eoi(keyboard_irq);
                sti();
                return;
            } else if (caps_lock == 0 && ascii_1[c] <= char_z && ascii_1[c] >= char_a) {
                putc(ascii_1[c]);
                send_eoi(keyboard_irq);
                sti();
                return;
            } else {
                putc(ascii_1[c]);
                send_eoi(keyboard_irq);
                sti();
                return;
            }
        }
    }
    send_eoi(keyboard_irq);
    sti();
    return;
}

/* void clear_buffer()
 *  Functionality: Clears the keyboard buffer
 *  Arguments: None
 *  Return: None
 ***********************************************************************************
 *  
 */
void clear_buffer() {
    int i;
    for(i = 0; i < BUFFER_LENGTH; i++) {
        keyboard_buffer[i] = 0x0; /*Set all buffer elements to null*/
    }
}

uint8_t get_curr_terminal() {
    return curr_terminal;
}

int get_kbd_buffer() {
    return kbd_buffer;
}

void set_kbd_buffer(int kbd_buf) {
    kbd_buffer = kbd_buf;
}
