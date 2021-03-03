/***********************************************************************
MODULE:    	FIFO BUFFER
VERSION:   	1.01
CONTAINS:  	FIFO buffer implementation for Rin-block elements
COPYRIGHT: 	Rinstrum Pty Ltd 2004
DATE: 		9 Mar 2004
NOTES:          Due to potential corruption, many of these functions will
                disable interrupts globally.
***********************************************************************/

#include <avr/io.h>
#include <avr/wdt.h>

#include "fifo.h"        /* FIFO buffer handler                       */


///////////////////////////////////////////////////////////////////////////////////////////
// Function 	: FifoFree
// Purpose	: Returns amount of free buffer space
// Parameters	: struct t_fifo_ctl *psBufferStruct
// Return Type	: int8u
// Notes:
//

uint8_t FifoFree(struct s_fifo_ctl *psBufferStruct)
{
  return psBufferStruct->ucFree;
}
