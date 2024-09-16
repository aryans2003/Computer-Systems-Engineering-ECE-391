#include "scheduling.h"
extern void flush_tlb();
uint8_t active_terminal = 1;
uint8_t terminals[3] = {0, 0, 0};
int num_shells = 0;

/* void init_PIT()
 *  Functionality: Sets up PIT interrupts and configs
 *  Arguments: None
 *  Return: None
 *  References: OSDev
 ***********************************************************************************
 *  
 */
void init_PIT() {
    outb(0x36, PIT_CMD_REG); //0x36 = 00110110b -> Channel 0, lobyte/hibyte, Rate Generator
    uint16_t reload_value = BASE_FREQ/100; //100 for a Time period of 10ms
    uint8_t low_byte = reload_value & 0xFF;        
    uint8_t high_byte = (reload_value >> 8) & 0xFF;
    outb(low_byte, PIT_CHANNEL0);
    outb(high_byte, PIT_CHANNEL0);
    enable_irq(PIT_irq);
}

/* void PIT_handler()
 *  Functionality: Interrupt handler for PIT interrupts
 *  Arguments: None
 *  Return: None
 *  References: None
 ***********************************************************************************
 *  
 */
void PIT_handler() {
    // test_interrupts();
    if(terminals[active_terminal-1] == 0) { // if new terminal
        terminals[active_terminal-1] = 1;
        terminal_switch(active_terminal);
        send_eoi(PIT_irq);
        clear();
        if (num_shells < 3) { // create 3 shell instances
            num_shells++;
            execute((uint8_t*)"shell");
        }
        if(active_terminal == 3) { // loop around
            terminal_switch(1);
        }
    }
    active_terminal++;
    if(active_terminal == 4) { // loop around
        active_terminal = 1;
        send_eoi(PIT_irq);
    }
    flush_tlb();
    send_eoi(PIT_irq);
}

/* uint8_t get_active_terminal()
 *  Functionality: Helper function to get active terminal
 *  Arguments: None
 *  Return: None
 *  References: None
 ***********************************************************************************
 *  
 */
uint8_t get_active_terminal() {
    return active_terminal;
}
