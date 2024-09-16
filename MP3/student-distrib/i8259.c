/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    master_mask = PIC_MASK;
    slave_mask = PIC_MASK;
 
    // starts the initialization sequence (in cascade mode)
	outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    // vector offsets for master and slave
	outb(ICW2_MASTER, MASTER_DATA);
	outb(ICW2_SLAVE, SLAVE_DATA);

    // connect master and slave with cascading
	outb(ICW3_MASTER, MASTER_DATA);
	outb(ICW3_SLAVE, SLAVE_DATA);

    // put PICs in 8086 mode
	outb(ICW4, MASTER_DATA);
	outb(ICW4, SLAVE_DATA);

    // restore masks
	outb(master_mask, MASTER_DATA);
	outb(slave_mask, SLAVE_DATA);

    enable_irq(CASCADE);    //Enabling the slave port interrupts
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    // enable for either slave or master
    if (irq_num <= SLAVE_MAX_IRQ && irq_num >= MASTER_MAX_IRQ) {
        irq_num -= MASTER_MAX_IRQ;
        slave_mask &= ~(1 << irq_num);
        outb(slave_mask, SLAVE_DATA);
    } else if (irq_num < MASTER_MAX_IRQ) {
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_DATA);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    // disable for either slave or master
    if (irq_num <= SLAVE_MAX_IRQ && irq_num >= MASTER_MAX_IRQ) {
        irq_num -= MASTER_MAX_IRQ;
        slave_mask |= (1 << irq_num);
        outb(slave_mask, SLAVE_DATA);
    } else if (irq_num < MASTER_MAX_IRQ) {
        master_mask |= (1 << irq_num);
        outb(master_mask, MASTER_DATA);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    // send for only master or both slave and master
    if (irq_num <= SLAVE_MAX_IRQ && irq_num >= MASTER_MAX_IRQ) {
		outb(EOI | (irq_num - MASTER_MAX_IRQ), SLAVE_8259_PORT);
        outb(EOI | CASCADE, MASTER_8259_PORT);
    } else if (irq_num < MASTER_MAX_IRQ) {
       outb(EOI | irq_num, MASTER_8259_PORT);
    }
}
