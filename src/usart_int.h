/*
 * usart_int.h
 *
 *  Created on: 16/09/2015
 *      Author: IanB
 */

#ifndef USART_INT_H_
#define USART_INT_H_

#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#include "fifo.h"


#define UART_GPIO_PORT(usart) ((usart == &USARTC0) ? &PORTC : (usart == &USARTD0) ? &PORTD : &PORTE)

/** Structure for interrupt driven USART */
struct s_usart
{
  volatile USART_t *hardware;
  struct s_fifo_ctl in_fifo;
  struct s_fifo_ctl out_fifo;
};


/** Allocate fifo buffers and usart structure */
#define ALLOCATE_USART_INT(name, infifosize, outfifosize) \
  static uint8_t name ## _infifobuf[infifosize]; \
  static uint8_t name ## _outfifobuf[outfifosize]; \
  struct s_usart name


/** Initialise USART with buffers allocated via ALLOCATE_USART_INT */
#define USART_INT_INIT(name, hw) \
  usart_int_init(&name, hw, name ## _infifobuf, sizeof(name ## _infifobuf), name ## _outfifobuf, sizeof(name ## _outfifobuf))


void set_end_character_dc4(struct s_usart *usart, bool status);
bool end_character_dc4(struct s_usart *usart);

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
void usart_int_init(struct s_usart *usart, USART_t *hardware, uint8_t *rxbuf, uint8_t rxbuf_size, uint8_t *txbuf, uint8_t txbuf_size);


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
void usart_int_configure(struct s_usart *usart, uint32_t baud, char parity, uint8_t databits, uint8_t stopbits);
void usart_int_internal_configure(struct s_usart *usart, uint32_t baud, char parity, uint8_t databits, uint8_t stopbits);


/** Send a character out the serial port */
bool usart_int_putchar(struct s_usart *usart, unsigned char ch);


/** Get a character from the serial port */
bool usart_int_getchar(struct s_usart *usart, uint8_t *ch);


/** Get the number of characters in the receive buffer */
int usart_int_getrxlevel(struct s_usart *usart);


/** Get if the receive buffer is full */
bool usart_int_getrxfull(struct s_usart *usart);


/** Flush the receive buffer */
void usart_int_flushrx(struct s_usart *usart);


/** Get last character from the serial port */
bool usart_int_getlastchar(struct s_usart *usart, uint8_t *ch);


#endif /* USART_INT_H_ */
