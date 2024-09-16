#include "rtc.h"

#define RTC_INDEX       0x70
#define CMOS            0x71
#define STATUS_REG_A    0X0A
#define STATUS_REG_B    0X0B
#define STATUS_REG_C    0X0C

/* void init_RTC()
 *  Functionality: Sets up RTC interrupts and configs
 *  Arguments: None
 *  Return: None
 *  References: OSDev
 ***********************************************************************************
 *  
 */

int rtc_int = 0;
int frequencies[10] = {0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06};

void init_RTC() {
    cli();
    /*Turning on IRQ8*/
    char prev;
    outb(STATUS_REG_B, RTC_INDEX);		        // select register B
    prev = inb(CMOS);	            // read the current value of register B
    outb(STATUS_REG_B, RTC_INDEX);		        // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, CMOS);	    // write the previous value ORed with 0x40. This turns on bit 6 of register B
    
    enable_irq(RTC_irq);
    sti();
}

/* void RTC_handler()
 *  Functionality: Interrupt handler for RTC interrupts
 *  Arguments: None
 *  Return: None
 *  References: OSDev
 ***********************************************************************************
 *  
 */
void RTC_handler() {
    /*Uncomment this to test RTC*/
    // test_interrupts();
    cli();
    
    outb(STATUS_REG_C, RTC_INDEX);	// select register C
    inb(CMOS);		    // just throw away contents

    rtc_int = 1;    // set interrupt

    send_eoi(RTC_irq);
    sti();
}

/* void change_RTC_rate()
 *  Functionality: Function to change the RTC interrupt frequency
 *  Arguments: rate -- the interrupt frequency rate that needs to be applied
 *  Return: -1 if it fails and 0 on a success
 *  References: OSDev
 ***********************************************************************************
 *  
 */
int change_RTC_rate(int rate) {
    if(rate == 0) {
        return -1;
    }
    cli();
    char prev;
    rate &= 0x0F;			// rate must be above 2 and not over 15
    outb(STATUS_REG_A, RTC_INDEX);		// set index to register A, disable NMI
    prev = inb(CMOS);	// get initial value of register A
    outb(STATUS_REG_A, RTC_INDEX);		// reset index to A
    outb((prev & 0xF0) | rate, CMOS); //write only our rate to A. Note, rate is the bottom 4 bits. 0xF0 is to clear the upper bits of prev
    sti();
    return 0;
}

/* void RTC_read()
 *  Functionality: RTC read function
 *  Arguments: fd -- file that is passed in
 *             buf -- buffer that is passed in
 *             nbytes -- number of bytes to be read
 *  Return: returns 0 on success
 *  References: OSDev
 ***********************************************************************************
 *  
 */
int32_t RTC_read (int32_t fd, void* buf, int32_t nbytes) {
    rtc_int = 0;
    while(rtc_int ==  0);
    return 0;
}

/* void RTC_read()
 *  Functionality: RTC write function
 *  Arguments: fd -- file that is passed in
 *             buf -- buffer that is passed in to be written
 *             nbytes -- number of bytes to be written
 *  Return: -1 if it fails and 0 on a success
 *  References: None
 ***********************************************************************************
 *  
 */
int32_t RTC_write (int32_t fd, const void* buf, int32_t nbytes) {
    cli();
    int new_rate = *((int*)buf);
    int rate = 0;
    switch(new_rate) { // check each rate option
        case 2: 
            rate = frequencies[0];
            break;
        case 4:
            rate = frequencies[1];
            break;
        case 8:
            rate = frequencies[2];
            break;
        case 16: 
            rate = frequencies[3];
            break;
        case 32:
            rate = frequencies[4];
            break;
        case 64:
            rate = frequencies[5];
            break;
        case 128: 
            rate = frequencies[6];
            break;
        case 256:
            rate = frequencies[7];
            break;
        case 512:
            rate = frequencies[8];
            break;
        case 1024:
            rate = frequencies[9];
            break;
        default:
            break;
    }
    sti();
    return change_RTC_rate(rate);
}

/* void RTC_open()
 *  Functionality: RTC open function
 *  Arguments: filename -- file that is passed in
 *  Return: returns 0
 *  References: None
 ***********************************************************************************
 *  
 */
int32_t RTC_open (const uint8_t* filename) {
    cli();
    int rate;
    rate = frequencies[0];
    change_RTC_rate(rate);
    sti();
    return 0;
}

/* void RTC_open()
 *  Functionality: RTC close function
 *  Arguments: fd -- file that is passed in
 *  Return: returns 0
 *  References: None
 ***********************************************************************************
 *  
 */
int32_t RTC_close (int32_t fd) {
    return 0;
}
