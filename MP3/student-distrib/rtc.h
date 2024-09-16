#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "i8259.h"
#include "lib.h"

/*RTC IRQ index*/
#define RTC_irq    8

/* Initialize the RTC */
void init_RTC();

/* Handles RTC interrupt */
void RTC_handler();

int change_RTC_rate(int rate);

int32_t RTC_read (int32_t fd, void* buf, int32_t nbytes);
int32_t RTC_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t RTC_open (const uint8_t* filename);
int32_t RTC_close (int32_t fd);

#endif /* _RTC_H */
