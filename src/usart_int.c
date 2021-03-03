/*
 * usart_int.c
 *
 *  Created on: 15/09/2015
 *      Author: IanB
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include "fifo.h"
#include "vectdata.h"

#include "usart_int.h"
#include "sci.h"
#include "asciidef.h"

bool endCharDC4_UC0 = false;
bool endCharDC4_UD0 = false;
bool endCharDC4_UE0 = false;


void usart_receive_interrupt(struct s_usart *usart)
{
	uint8_t uData = 0;
  if (NULL == usart)
    return;

  /* Clear receive errors */
  if (usart->hardware->STATUS & (USART_FERR_bm | USART_BUFOVF_bm | USART_PERR_bm))
    usart->hardware->STATUS &= ~(USART_FERR_bm | USART_BUFOVF_bm | USART_PERR_bm);

  /* Put received data into fifo */
  uData = (uint8_t)usart->hardware->DATA; // data auto clear once you read it
  if((usart->hardware == &USARTD0) && (uData & SPECIAL_CHAR_MASK) == SPECIAL_CHAR_MASK)
  	put_char_sc_buffer(&specialCH, uData);
  	//put_char_sc_buffer(&specialCH, usart->hardware->DATA);
  else
  {
  	if((uData == H_DC2) && !end_character_dc4(usart)) // if detect a DC2 character then message end character should alter to DC4
  		set_end_character_dc4(usart,true);

  	FifoPutChar(&usart->in_fifo, uData);
  }
}


void usart_dataempty_interrupt(struct s_usart *usart)
{
  uint8_t ch;

  if (NULL == usart)
    return;

  /* Is there any more data to send? */
  if (FifoGetChar(&usart->out_fifo, &ch))
  {
    usart->hardware->DATA = ch;
  }
  else
  {
    /* Disable the data register empty interrupt */
    usart->hardware->CTRLA &= ~USART_DREINTLVL_gm;
  }
}


#if 0
void usart_transmitcomplete_interrupt(struct s_usart *usart)
{
  /* Not required */
}
#endif


#define DEFINE_USART_ISRS(which) \
void which ## _RXC_vect (void) __attribute__ ((signal,__INTR_ATTRS)); \
void which ## _RXC_vect (void) \
{ \
  usart_receive_interrupt((struct s_usart *) get_data_for_vector(which ## _RXC_vect_num)); \
} \
\
void which ## _DRE_vect (void) __attribute__ ((signal,__INTR_ATTRS)); \
void which ## _DRE_vect (void) \
{ \
  usart_dataempty_interrupt((struct s_usart *) get_data_for_vector(which ## _DRE_vect_num)); \
}

#if 0
\
void which ## _TXC_vect (void) __attribute__ ((signal,__INTR_ATTRS)); \
void which ## _TXC_vect (void) \
{ \
  usart_transmit_interrupt((struct s_usart *) get_data_for_vector(which ## _TXC_vect_num)); \
}
#endif


DEFINE_USART_ISRS(USARTC0)
DEFINE_USART_ISRS(USARTD0)
DEFINE_USART_ISRS(USARTE0)


#define BAUD_TO_BAUDCTRL(baud, freq, bscale) ((freq / ((1 << bscale) * 16) / baud) - 1)
#define BAUD_TO_BAUDCTRL_CLK2X(baud, freq, bscale) ((freq / ((1 << bscale) * 8) / baud) - 1)

#define FREQ 18432000UL
#define BSCALE 0
#define BACALE_LOW_BAUD	2

#define BAUD_TO_BAUDCTRL_LOW_BAUD(baud) BAUD_TO_BAUDCTRL(baud, FREQ, BACALE_LOW_BAUD)
#define BAUD_TO_BAUDCTRL_FIXED(baud) BAUD_TO_BAUDCTRL(baud, FREQ, BSCALE)
#define BAUD_TO_BAUDCTRL_FIXED_CLK2X(baud) BAUD_TO_BAUDCTRL_CLK2X(baud, FREQ, BSCALE)


bool end_character_dc4(struct s_usart *usart)
{
	bool status = false;

	if(usart->hardware == &USARTC0)
		status = endCharDC4_UC0;
	else if(usart->hardware == &USARTD0)
		status = endCharDC4_UD0;
	else
		status =endCharDC4_UE0;

	return status;
}

void set_end_character_dc4(struct s_usart *usart, bool status)
{
	if(usart->hardware == &USARTC0)
		endCharDC4_UC0 = status;
	else if(usart->hardware == &USARTD0)
		endCharDC4_UD0 = status;
	else
		endCharDC4_UE0 = status;
}


/**
 * Initialise usart as specified in the parameters
 *
 * @param usart is the usart structure to initialise
 * @param hardware is usart hardware pointer
 * @param rxbuf is the receive buffer
 * @param rxbuf_size is the size of the receive buffer
 * @param txbuf is the transmit buffer
 * @param txbuf_size is the size of the transmit buffer
 */
void usart_int_init(struct s_usart *usart, USART_t *hardware, uint8_t *rxbuf, uint8_t rxbuf_size, uint8_t *txbuf, uint8_t txbuf_size)
{
  usart->hardware = hardware;
  FifoInit(&usart->in_fifo, rxbuf, rxbuf_size);
  FifoInit(&usart->out_fifo, txbuf, txbuf_size);
}


/**
 * Initialise usart as specified in the parameters
 *
 * @param usart is the hardware uart to use
 * @param baud is the baud rate
 * @param parity is 'n', 'o' or 'e'
 * @param databits is 5, 6, 7, 8, 9
 * @param stopbits is 1, 2
 *
 */
void usart_int_configure(struct s_usart *usart, uint32_t baud, char parity, uint8_t databits, uint8_t stopbits)
{
  const uint8_t chsize[] = { USART_CHSIZE_5BIT_gc, USART_CHSIZE_6BIT_gc, USART_CHSIZE_7BIT_gc, USART_CHSIZE_8BIT_gc, USART_CHSIZE_9BIT_gc };
  uint8_t ctrlc;

  /* Disable transmit & receive */
  usart->hardware->CTRLB = 0x00;

  /* Enable receive interrupt */
  usart->hardware->CTRLA = (USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_OFF_gc | USART_DREINTLVL_OFF_gc);

  /* Set Async, parity, data and stop bits */
  ctrlc = USART_CMODE_ASYNCHRONOUS_gc;
  ctrlc |= ('n' == parity) ? USART_PMODE_DISABLED_gc : ('e' == parity) ? USART_PMODE_EVEN_gc : USART_PMODE_ODD_gc;
  ctrlc |= (1 == stopbits) ? (0 << USART_SBMODE_bp) : (1 << USART_SBMODE_bp);
  ctrlc |= chsize[databits - 5];
  usart->hardware->CTRLC = ctrlc;

  /* Set baud with CLK2X = 0 (BSEL = fper/(2^bscale*16*fbaud) - 1 */
  if(baud >= 4800)
  {
  	usart->hardware->BAUDCTRLA = BAUD_TO_BAUDCTRL_FIXED(baud);
  	usart->hardware->BAUDCTRLB = (0 << USART_BSCALE_gp) | 0;
  }
  else
  {
  	usart->hardware->BAUDCTRLA = BAUD_TO_BAUDCTRL_LOW_BAUD(baud);
  	usart->hardware->BAUDCTRLB = (BACALE_LOW_BAUD << USART_BSCALE_gp) | 0;
  }

  /* Setup IO: TX pin as output, RX pin as input */
  UART_GPIO_PORT(usart->hardware)->DIRSET = (1 << 3);
  UART_GPIO_PORT(usart->hardware)->DIRCLR = (1 << 2);

  /* Enable transmit and receive */
  usart->hardware->CTRLB = (USART_RXEN_bm | USART_TXEN_bm | 0 << USART_CLK2X_bp);
}

// this is to configure special baud rates (51200, 102400, 153600) same as "usart_int_configure(...);" function
void usart_int_internal_configure(struct s_usart *usart, uint32_t baud, char parity, uint8_t databits, uint8_t stopbits)
{
  const uint8_t chsize[] = { USART_CHSIZE_5BIT_gc, USART_CHSIZE_6BIT_gc, USART_CHSIZE_7BIT_gc, USART_CHSIZE_8BIT_gc, USART_CHSIZE_9BIT_gc };
  uint8_t ctrlc;

  /* Disable transmit & receive */
  usart->hardware->CTRLB = 0x00;

  /* Enable receive interrupt */
  usart->hardware->CTRLA = (USART_RXCINTLVL_LO_gc | USART_TXCINTLVL_OFF_gc | USART_DREINTLVL_OFF_gc);

  /* Set Async, parity, data and stop bits */
  ctrlc = USART_CMODE_ASYNCHRONOUS_gc;
  ctrlc |= ('n' == parity) ? USART_PMODE_DISABLED_gc : ('e' == parity) ? USART_PMODE_EVEN_gc : USART_PMODE_ODD_gc;
  ctrlc |= (1 == stopbits) ? (0 << USART_SBMODE_bp) : (1 << USART_SBMODE_bp);
  ctrlc |= chsize[databits - 5];
  usart->hardware->CTRLC = ctrlc;

  /* Set baud with CLK2X = 1 (BSEL = fper/(2^bscale*8*fbaud) - 1 */
  usart->hardware->BAUDCTRLA = BAUD_TO_BAUDCTRL_FIXED_CLK2X(baud);
  usart->hardware->BAUDCTRLB = (0 << USART_BSCALE_gp) | 0;

  /* Setup IO: TX pin as output, RX pin as input */
  UART_GPIO_PORT(usart->hardware)->DIRSET = (1 << 3);
  UART_GPIO_PORT(usart->hardware)->DIRCLR = (1 << 2);

  /* Enable transmit and receive */
  usart->hardware->CTRLB = (USART_RXEN_bm | USART_TXEN_bm | 1 << USART_CLK2X_bp); // set double speed clock
}


/** Send a character out the serial port */
bool usart_int_putchar(struct s_usart *usart, unsigned char ch)
{
  bool result;
  uint8_t sreg;
  uint8_t ctrla;

  sreg = SREG;
  SREG = 0;

  ctrla = usart->hardware->CTRLA;
  if ((ctrla & USART_DREINTLVL_gm) == USART_DREINTLVL_OFF_gc)
  {
    /* Not currently transmitting, so just write to transmit register and enable DRE interrupt */
    usart->hardware->DATA = ch;
    ctrla &= ~USART_DREINTLVL_gm;
    ctrla |= USART_DREINTLVL_LO_gc;
    usart->hardware->CTRLA = ctrla;
    result = true;
  }
  else
  {
    /* Currently transmitting, so store into fifo */
    result = FifoPutChar(&usart->out_fifo, ch);
  }

  SREG = sreg;
  return result;
}


/** Get a character from the serial port */
bool usart_int_getchar(struct s_usart *usart, uint8_t *ch)
{
  if (NULL == usart || NULL == ch)
    return false;

  return FifoGetChar(&usart->in_fifo, ch);
}


/** Get the number of characters in the receive buffer */
int usart_int_getrxlevel(struct s_usart *usart)
{
  if (NULL == usart)
    return 0;
  return FifoUsed(&usart->in_fifo);
}


/** Get if the receive buffer is full */
bool usart_int_getrxfull(struct s_usart *usart)
{
  if (NULL == usart)
    return 0;
  return (FifoFree(&usart->in_fifo) == 0);
}


/** Flush the receive buffer */
void usart_int_flushrx(struct s_usart *usart)
{
  if (NULL == usart)
      return;
  FifoFlushBuffer(&usart->in_fifo);
}

/** Get last character from the serial port */
bool usart_int_getlastchar(struct s_usart *usart, uint8_t *ch)
{
  if (NULL == usart || NULL == ch)
    return false;

  return FifoGetLastChar(&usart->in_fifo, ch);
}
