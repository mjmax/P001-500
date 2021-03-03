/*
 * outputs.c
 *
 *  Created on: 12/10/2015
 *      Author: IanB
 */
#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>

#include "outputs.h"


/*
 * OVER = OUT1 = PB0
 * UNDER = OUT2 = PB1
 * PASS = OUT3 = PB2
 *
 * OVER = OUT1LED = PB4
 * UNDER = OUT2LED = PB5
 * PASS = OUT3LED = PB6
 *
 */


void outputs_init(void)
{
  PORTB.PIN0CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.PIN1CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.PIN2CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.PIN3CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.PIN4CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.PIN5CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.PIN6CTRL = (0 << PORT_INVEN_bp | PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc);
  PORTB.DIRSET = (1 << 6 | 1 << 5 | 1 << 4 | 1 << 3 | 1 << 2 | 1 << 1 | 1 << 0);
}


void outputs_set(uint8_t output, bool level)
{
  if (level)
    PORTB.OUTSET = 1 << output;
  else
    PORTB.OUTCLR = 1 << output;
}
