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
// Function 	: FifoInit
// Purpose	: Initialises buffer
// Parameters	: struct t_fifo_ctl *psBufferStruct
//                int8u *pucBuffer
//                int8u ucBufferSize
// Return Type	: void
// Notes:
//                !!Disables interrupts globally!!
//

void FifoInit(struct s_fifo_ctl *psBufferStruct, uint8_t *pucBuffer, uint8_t ucBufferSize)
{
  uint8_t sReg;

  if (ucBufferSize > 254)
  {
    ucBufferSize = 254;
  }

  /* disable interrupts globally */
  sReg = SREG;
  SREG = 0;

  psBufferStruct->pucBuffer = pucBuffer;
  psBufferStruct->ucBufferSize = ucBufferSize;
  psBufferStruct->ucFree = ucBufferSize;
  psBufferStruct->ucHeadIndex = 0;
  psBufferStruct->ucTailIndex = 0;

  /* reenable interrupts globally */
  SREG = sReg;
}
