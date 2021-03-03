/*
 * hardware.h
 *
 *  Created on: 18/09/2015
 *      Author: IanB
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdio.h>
#include <stdbool.h>


extern FILE usart2_stream;
extern FILE usart3_stream;
extern FILE usart1_stream;
extern FILE usart0_stream;

#define USART_RX_BUFSIZE 254
#define USART_TX_BUFSIZE 254

/** Available baud rate settings */
typedef enum
{
	DB_SERIAL_BAUDIDX_2400 = 0,
	DB_SERIAL_BAUDIDX_4800,
  DB_SERIAL_BAUDIDX_9600,
  DB_SERIAL_BAUDIDX_19200,
  DB_SERIAL_BAUDIDX_38400,
  DB_SERIAL_BAUDIDX_57600,
  DB_SERIAL_BAUDIDX_115200,

  NUM_DB_BAUD_SETTINGS
}eSerialBaudIndex;

typedef enum
{
  DB_SERIAL_PARITYIDX_NONE = 0,
  DB_SERIAL_PARITYIDX_EVEN,
  DB_SERIAL_PARITYIDX_ODD,

  NUM_DB_PARITY_SETTINGS
}eSerialParityIndex;

typedef enum
{
  DB_SERIAL_DATAIDX_5 = 0,
  DB_SERIAL_DATAIDX_6,
  DB_SERIAL_DATAIDX_7,
  DB_SERIAL_DATAIDX_8,

  NUM_DB_DATA_SETTINGS
}eSerialDataBitsIndex;


typedef enum
{
  DB_SERIAL_STOPIDX_1 = 0,
  DB_SERIAL_STOPIDX_2,

  NUM_DB_STOP_SETTINGS
}eSerialStopBitsIndex;

/* Sed the stream to the usart */
int fdev_putchar(char ch, FILE *stream);

/** Get the character available in the stream */
int fdev_getchar(FILE *stream);

/** Get the number of characters available in the stream */
int fgetbufferlevel(FILE *stream);


/** Get if there is a entire message in the stream */
uint8_t fgetmessage(FILE *stream);


/** Check if the buffer is full, flush if so */
void fcheckbufferfull(FILE *stream);

//** get serial baud rate from serial baud rate index */
uint32_t get_serial_baud_rate(uint8_t baudRateIndex);

/**Check for serial baud rate available on the list*/
bool isSerialBaudAvailable(uint32_t tempBaud, uint8_t *index);

//** get serial parity from serial parity index */
char get_serial_parity(uint8_t parityIndex);

/**Check for parity available on the list*/
bool isSerialParityAvailable(char tempParity, uint8_t *index);

//** get serial data bit from serial data bit index */
uint8_t get_serial_data_bit(uint8_t dataBitIndex);

/**Check for data bits available on the list*/
bool isSerialDataBitAvailable(char tempDataBit, uint8_t *index);

//** get serial stop bit from serial stop bit index */
uint8_t get_serial_stop_bit(uint8_t dataBitIndex);

/**Check for stop bits available on the list*/
bool isSerialStopBitAvailable(char tempStopBit, uint8_t *index);

/** initialize usart hardware */
void init_usart(void);

/** Initialise the hardware */
void init_hardware(void);

/*change usart settings of RS232 port*/
void ChangeExternalUsartSettings(double externalBoud,char parity, int dataBits, int stopBit);

/*change usart settings of Blue tooth port*/
void ChangeBtUsartSettings(double btBoud,char parity, int dataBits, int stopBit);

/*check the baud rate is available*/
bool isBaudRate(double baud);

#endif /* HARDWARE_H_ */
