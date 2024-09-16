/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

int spamming = 0; // set global variable to check for spamming buttons
char input[6]; // buffer to send data (Opcode + Active + Display 1 - 4 = 6 bytes)
char packet1; // var to hold packet[1] data
char packet2; // var to hold packet[2] data
char reset[6]; // buffer to handle reset (Opcode + Active + Display 1 - 4 = 6 bytes)

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/************************ Protocol Implementation *************************/

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
	/* Given code */
    unsigned a, b, c;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

	/* Dispatcher implementation */
	switch(a) // we check the value at packet[0]
	{
		case MTCP_BIOC_EVENT: // when the Button Interrupt-on-change mode is enabled and a button is either pressed or released.
			packet1 = b; // save value of packet[1]
			packet2 = c; // save value of packet[2]
		case MTCP_ACK: // when MTC successfully completes a command
			spamming = 1; // set spamming flag
			break;
		case MTCP_RESET: // device re-initializes itself after a power-up, a RESET button press, or an MTCP_RESET_DEV command.
			tuxctl_initialize(tty); // re-initialize tux controller
			tuxctl_ldisc_put(tty, reset, 6); // Writes 6 commands out to device
			break;
		default:
			break;
	}

    /*printk("packet : %x %x %x\n", a, b, c); */
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) { // check passed command
	case TUX_INIT:
		return tuxctl_initialize(tty); // if TUX_INIT, we move to defined initialize funct
		return 0;
	case TUX_BUTTONS:
		return tuxctl_buttons(arg); // if TUX_BUTTONS, we move to our buttons funct
		return 0;
	case TUX_SET_LED:
		if(!spamming) // check to ensure the user is not spamming buttons
		{
			return -EINVAL; // if they are, return an error
		}
		return tuxctl_LED(tty, arg); // if not spamming, we move to our LED funct
		return 0;
	case TUX_LED_ACK:
		return -EINVAL; // cmd invalid
	case TUX_LED_REQUEST:
		return -EINVAL; // cmd invalid
	case TUX_READ_LED:
	default:
	    return -EINVAL; // cmd invalid
    }
}

/*
 * tuxtcl_initialize
 *   DESCRIPTION: Initialize TuxController device
 *   INPUTS: struct tty_struct* tty - pointer to device struct
 *   OUTPUTS: none
 *   RETURN VALUE: 0 (if successful)
 *   SIDE EFFECTS: Sends commands to enable Button Interrupt-on-change mode and set LED display to user mode
 */
int tuxctl_initialize(struct tty_struct* tty)
{
	input[0] = MTCP_BIOC_ON; // enable Button Interrupt-on-change mode
	input[1] = MTCP_LED_USR; // set LED display to user mode
	tuxtcl_ldisc_put(tty, input, 2); // write 2 commands out to the device
	return 0; // inidicate succesful initialization
}

/*
 * tuxtcl_buttons
 *   DESCRIPTION: Read button states from TuxController and return info
 *   INPUTS: unsigned long arg - user-space pointer that holds button info
 *   OUTPUTS: none
 *   RETURN VALUE: 0 (if successful), else -EINVAL
 *   SIDE EFFECTS: Reads button states from arg and copies info to user
 */
int tuxctl_buttons(unsigned long arg)
{
	unsigned char byte_packets = 0x0; // set to 0x0 to force to hex mode
	/* If invalid input, we return an error */
	if((unsigned long*)(arg) == NULL)
	{
		return -EINVAL;
	}
	/* using an interrupt-driven approach to set bits of low byte 
	according to the currently pressed buttons */

	byte_packets |= ((packet2 & 0x08) == 0x08) << 3; // Button 3 (right) is pressed
    byte_packets |= ((packet2 & 0x02) == 0x02) << 2; // Button 2 (down) is pressed
    byte_packets |= ((packet2 & 0x04) == 0x04) << 1; // Button 1 (left) is pressed
    byte_packets |= ((packet2 & 0x01) == 0x01) << 0; // Button 0 (up) is pressed

	byte_packets <<= 4; // shift left by 4
    byte_packets |= (packet1 & 0x0F); // add lower 4 bits of packet[0]

	/* Copy button info to user-space using copy_to_user and return success */
	if(!copy_to_user((unsigned long*)(arg), &byte_packets, sizeof(byte_packets)))
	{
		return 0;
	}
	return -EINVAL; // else, error
}

/*
 * tuxtcl_LED
 *   DESCRIPTION: Sets LEDs on Tux Controller, sends configuration for display
 *   INPUTS: struct tty_struct* tty - pointer to Tux Controller interface
 * 			 unsigned long arg - argument for the LED configuration
 *   OUTPUTS: Sends LED configuration to Tux Controller
 *   RETURN VALUE: 0 (if successful)
 *   SIDE EFFECTS: Modifies LED configuration based on input arg and sends
 * 				   its resulting display to Tux Controller
 */
int tuxctl_LED(struct tty_struct* tty, unsigned long arg)
{
	int i; // init var for loop
	int j; // init var for loop
	int k; // init var for loop
	/* Define LED values */
	unsigned char LED[17] = 
	{0xE7, 0x06, 0xCB, 0x8F, // LEDs 0, 1, 2, and 3
	 0x2E, 0xAD, 0xED, 0x86, // LEDs 4, 5, 6, and 7
	 0xEF, 0xAE, 0xEE, 0x6D,// LEDs 8, 9, A, and B
	 0xE1, 0x4F, 0xE9, 0xE8, 0x00}; // LEDs C, D, E, F, (blank)

	int offset = 2; // starting index for LED configuration
	input[0] = MTCP_LED_SET; // indicate LED set opcode
	input[1] = 0xFF; // set each LED to high

	for(i = 0; i < 4; i++) // loop thru 4 LEDs
	{
		/* Set LED configuration based on nibbles of argument */
		input[offset] = LED[0xF & arg];
		arg >>= 4;
		offset++;
	}

	for(j = 0; j < 4; j++)
	{
		/* Set each 4 LED to a blank configuration if 
		   corresponding bit is blank */
		if(!(arg & 0x01))
		{
			input[offset++] = LED[16];
		}
		arg >>= 1;
	}
	arg >>= 4;

	for(k = 0; k < 4; k++)
	{
		/* Set to configuration if not blank */
		if(arg & 0x01)
		{
			input[offset++] |= 0x10;
		}
		arg >>= 1;
	}

	memcpy(reset, input, 6); // save current configuration in reset array 

	tuxctl_ldisc_put(tty, input, 6); // send LED config to Tux controller
	return 0; // return success
}
