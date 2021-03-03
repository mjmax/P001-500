/*
 * r300.c
 *
 *  Created on: 12/10/2015
 *      Author: IanB
 */
#include <avr/io.h>
#include <stdio.h>
#include "inputs.h"
#include "outputs.h"
#include "r300.h"
#include "sci.h"
#include "command.h"


void handle_r300_data(FILE *stream, uint8_t ch)
{
  fputc(ch, stream);
}

// this function sync data with R320 when special flag pop up
void AutomaticSyncData(void)
{
	eSciPort tempPort;

	if(GetInputInterruptFlag()) // in case of input interrupt flag (change of input)
	{
//		outputs_set(OUT1, ((PORTA.IN & (1 << 4)) != 0) ? 1 : 0);
//		outputs_set(OUT2, ((PORTA.IN & (1 << 5)) != 0) ? 1 : 0);

		if(appVariables.status.inputStatus != (uint8_t)(PORTA.IN & 0x0F))
		{
			appVariables.status.inputStatus = (uint8_t)(PORTA.IN & 0x0F);
			tempPort = sci_get_current_port();
			sci_set_current_port(SCI_PORT_INTERNAL); // temporary set the current SCI port as internal
			CommandHandle("1RA:\r\n"); // process the read input command to send out the latest input value
			sci_set_current_port(tempPort); // reset set the current SCI port as previous
		}

		SetInputInterruptFlag(false); // set the flag false to avoid looping
	}
}
