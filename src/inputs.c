/*
 * inputs.c
 *
 *  Created on: 12/10/2015
 *      Author: IanB
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include "vectdata.h"
#include "outputs.h"
#include "database.h"
#include "inputs.h"
#include "clock.h"


/*
PA0 = INP1
PA1 = INP2
PA2 = INP3
PA3 = INP4

PA4 = IN1
PA5 = IN2
*/
uint8_t in1 = 0, in2 = 0, inp1 = 0, inp2 = 0, inp3 = 0, inp4 = 0;

uint16_t clockPluse1Start = 0;
uint16_t clockPluse2Start = 0;
uint16_t clockPluse3Start = 0;
uint16_t clockPluse4Start = 0;

uint16_t pulse1WidthMicroSec = 0;
uint16_t pulse2WidthMicroSec = 0;
uint16_t pulse3WidthMicroSec = 0;
uint16_t pulse4WidthMicroSec = 0;

bool inputInterrupt = false;

bool GetInputInterruptFlag(void)
{
	return inputInterrupt;
}

void SetInputInterruptFlag(bool flag)
{
	inputInterrupt = flag;
}

uint16_t getPulseWidth(uint8_t pulseName)
{
	uint16_t pulseWidth;

	if(pulseName >= MAX_PULSE)
		return 0;

	switch(pulseName)
	{
		case PULSE_1:
			pulseWidth = pulse1WidthMicroSec;
			break;
		case PULSE_2:
			pulseWidth = pulse2WidthMicroSec;
			break;
		case PULSE_3:
			pulseWidth = pulse3WidthMicroSec;
			break;
		case PULSE_4:
			pulseWidth = pulse4WidthMicroSec;
			break;
	}
	return pulseWidth;
}

void setPulseWidth(uint8_t pulseName, uint16_t pulseWidth)
{
	if(pulseName < MAX_PULSE)
	{
		switch(pulseName)
		{
			case PULSE_1:
				pulse1WidthMicroSec = pulseWidth;
				break;
			case PULSE_2:
				pulse2WidthMicroSec  = pulseWidth;
				break;
			case PULSE_3:
				pulse3WidthMicroSec  = pulseWidth;
				break;
			case PULSE_4:
				pulse4WidthMicroSec = pulseWidth;
				break;
		}
	}
}

ISR(PORTA_INT0_vect)
{
  FILE *stream;
  static uint8_t pulseIn = 0;

  stream = get_data_for_vector(PORTA_INT0_vect_num);
  if (NULL != stream)
  {
    // Read the values from the port
  	if((PORTA.IN & (1 << 0)) != 0)
  	{
  		pulseIn |= (uint8_t)(1 << 0);
  		clockPluse1Start = clockMicroSec();
  	}
  	else
  	{
  		if((pulseIn & (1 << 0)) != 0)
  		{
  			pulseIn &= ~(1 << 0);
  			setPulseWidth(PULSE_1, (clockMicroSec() - clockPluse1Start));
  		}
  	}

  	if((PORTA.IN & (1 << 1)) != 0)
  	{
  		pulseIn |= (uint8_t)(1 << 1);
  		clockPluse1Start = clockMicroSec();
  	}
  	else
  	{
  		if((pulseIn & (1 << 1)) != 0)
  		{
  			pulseIn &= ~(1 << 1);
  			setPulseWidth(PULSE_2, (clockMicroSec() - clockPluse2Start));
  		}
  	}

  	if((PORTA.IN & (1 << 2)) != 0)
  	{
  		pulseIn |= (uint8_t)(1 << 2);
  		clockPluse1Start = clockMicroSec();
  	}
  	else
  	{
  		if((pulseIn & (1 << 2)) != 0)
  		{
  			pulseIn &= ~(1 << 2);
  			setPulseWidth(PULSE_3, (clockMicroSec() - clockPluse3Start));
  		}
  	}

  	if((PORTA.IN & (1 << 3)) != 0)
  	{
  		pulseIn |= (uint8_t)(1 << 3);
  		clockPluse1Start = clockMicroSec();
  	}
  	else
  	{
  		if((pulseIn & (1 << 3)) != 0)
  		{
  			pulseIn &= ~(1 << 3);
  			setPulseWidth(PULSE_4, (clockMicroSec() - clockPluse4Start));
  		}
  	}

  	SetInputInterruptFlag(true);
  }
  PORTA.INTFLAGS |= PORT_INT0IF_bm;
}

void init_read_port_A(void)
{
  FILE *stream;

  stream = get_data_for_vector(PORTA_INT0_vect_num);
  if (NULL != stream)
  {
    // Read the values from the port
  	inp1 = ((PORTA.IN & (1 << 0)) != 0) ? 1 : 0;
  	inp2 = ((PORTA.IN & (1 << 1)) != 0) ? 1 : 0;
  	inp3 = ((PORTA.IN & (1 << 2)) != 0) ? 1 : 0;
  	inp4 = ((PORTA.IN & (1 << 3)) != 0) ? 1 : 0;
  }
  PORTA.INTFLAGS |= PORT_INT0IF_bm;
}

// Binary encoder from in to out
// 00 -> 0
// 01 -> 1
// 10 -> 2
// 11 -> 3
// Disable interrupts to prevent race conditions.
input_t get_inputs(void)
{
  input_t encoded;
  cli();
  encoded.internal = in2 * 2 + in1;
  encoded.external = inp4*8  + inp3*4  + inp2*2  + inp1*1;
  sei();

  return encoded;
}



void inputs_init(void)
{
  /* PA 4 & 5 as inputs, with interrupt generated on both edges */
  PORTA.DIRCLR = (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
  PORTA.INT0MASK |= (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);

  PORTA.PIN0CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTA.PIN1CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTA.PIN2CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTA.PIN3CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);

  PORTA.INTCTRL |= PORT_INT0LVL_LO_gc;
}
