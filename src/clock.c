/*
 * clock.c
 *
 *  Created on: Mar 3, 2021
 *      Author: Janaka
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <stdio.h>
#include "clock.h"

bool timeOut = false;
uint16_t microCount = 0;

void IntTimers(void)
{
	initMicroSecTimer();
}

//////////////////////////////////////////////////////
/*
 * TCD0 Timer use for read pulse width
 * set TCD0.PER for different value for different time interval.
 * TCD0.PER = 17 for 1 microsecond time interval
 * should call IntTimer before calling any method
 *
 */
void initMicroSecTimer(void)
{
	TCD0.CTRLA=TC_CLKSEL_OFF_gc;
	TCD0.CTRLB=TC_WGMODE_NORMAL_gc;		// counter in normal operation
	TCD0.INTCTRLA=TC_OVFINTLVL_HI_gc;
	TCD0.PER = 17; //17 for 1.024 micro second
	TCD0.CTRLA=TC_CLKSEL_DIV1_gc;
}

uint16_t clockMicroSec(void)
{
	return microCount;
}

ISR(TCD0_OVF_vect){

	microCount++;
	TCD0.CTRLA=TC_CLKSEL_OFF_gc;
	TCD0.PER = 17; //17 for 1.024 micro second
	TCD0.CTRLA=TC_CLKSEL_DIV1_gc;
}


//////////////////////////////////////////////////////
/*
 * TCC0 Timer use for system time critical functions drive
 * set TCC0.PER for different value for different time interval.
 * TCC0.PER = 71 for 1ms time intercal
 * should call IntTimer before calling any method
 *
 */
void IntSystemTimer(void)
{
	TCC0.CTRLA = TC_CLKSEL_OFF_gc;
	TCC0.CTRLB = TC_WGMODE_NORMAL_gc;		// counter in normal operation
	TCC0.INTCTRLA = TC_OVFINTLVL_HI_gc;
	TCC0.PER = 71; //1 millisecond (tick) (PRE = (fclock/(N*freq)) - 1)
	TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
	set_time_out(false);
}

void reset_timer(void)
{
	TCC0.CTRLA = TC_CLKSEL_OFF_gc;
	TCC0.PER = 71;
	TCC0.CTRLA = TC_CLKSEL_DIV256_gc;
}

void set_time_out(bool status)
{
	timeOut = status;
}

bool get_time_out_status(void)
{
	return timeOut;
}

ISR(TCC0_OVF_vect)
{
	set_time_out(true);
	reset_timer();
}

