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


/*
PA0 = INP1
PA1 = INP2
PA2 = INP3
PA3 = INP4

PA4 = IN1
PA5 = IN2
*/
uint8_t in1 = 0, in2 = 0, inp1 = 0, inp2 = 0, inp3 = 0, inp4 = 0;
bool inputInterrupt = false;

bool GetInputInterruptFlag(void)
{
	return inputInterrupt;
}

void SetInputInterruptFlag(bool flag)
{
	inputInterrupt = flag;
}

ISR(PORTA_INT0_vect)
{
  FILE *stream;
  uint8_t ioDef = (uint8_t)(((~userDBs.ioDirection) >> 4) & POART_A_MAX_INP);

  stream = get_data_for_vector(PORTA_INT0_vect_num);
  if (NULL != stream)
  {
    // Read the values from the port
  	if((ioDef & (1 << 0)) > 0)	// if PA0 is configured as input
  		inp1 = ((PORTA.IN & (1 << 0)) != 0) ? 1 : 0;
  	if((ioDef & (1 << 1)) > 0)	// if PA1 is configured as input
  		inp2 = ((PORTA.IN & (1 << 1)) != 0) ? 1 : 0;
		inp3 = ((PORTA.IN & (1 << 2)) != 0) ? 1 : 0;
		inp4 = ((PORTA.IN & (1 << 3)) != 0) ? 1 : 0;
		//in1 = ((PORTA.IN & (1 << 4)) != 0) ? 1 : 0;
		//in2 = ((PORTA.IN & (1 << 5)) != 0) ? 1 : 0;
		SetInputInterruptFlag(true);
  }
  PORTA.INTFLAGS |= PORT_INT0IF_bm;
}

void init_read_port_A(void)
{
  FILE *stream;
  uint8_t ioDef = (uint8_t)(((~userDBs.ioDirection) >> 4) & POART_A_MAX_INP);

  stream = get_data_for_vector(PORTA_INT0_vect_num);
  if (NULL != stream)
  {
    // Read the values from the port
  	if((ioDef & (1 << 0)) > 0)	// if PA0 is configured as input
  		inp1 = ((PORTA.IN & (1 << 0)) != 0) ? 1 : 0;
  	if((ioDef & (1 << 1)) > 0)	// if PA1 is configured as input
  		inp2 = ((PORTA.IN & (1 << 1)) != 0) ? 1 : 0;
		inp3 = ((PORTA.IN & (1 << 2)) != 0) ? 1 : 0;
		inp4 = ((PORTA.IN & (1 << 3)) != 0) ? 1 : 0;
		//in1 = ((PORTA.IN & (1 << 4)) != 0) ? 1 : 0;
		//in2 = ((PORTA.IN & (1 << 5)) != 0) ? 1 : 0;
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
  //PORTA.DIRCLR = (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 /*| 1 << 4 | 1 << 5*/);
  //PORTA.INT0MASK |= (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5);
	uint8_t ioDef = (uint8_t)(((~userDBs.ioDirection) >> 4) & POART_A_MAX_INP);

  PORTA.DIRCLR = ioDef;
  PORTA.INT0MASK |= ioDef;
  if((ioDef & (1 << 0)) > 0)	// if PA0 is configured as input
  	PORTA.PIN0CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  if((ioDef & (1 << 1)) > 0)	// if PA1 is configured as input
  	PORTA.PIN1CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTA.PIN2CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTA.PIN3CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  //PORTA.PIN4CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  //PORTA.PIN5CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTA.INTCTRL |= PORT_INT0LVL_LO_gc;
}
