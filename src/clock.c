/*
 * clock.c
 *
 *  Created on: Mar 3, 2021
 *      Author: Janaka
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "clock.h"

uint16_t microCount = 0;

void IntTimers(void)
{
	initMicroSecTimer();
}

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

