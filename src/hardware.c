/*
 * hardware.c
 *
 *  Created on: 17/09/2015
 *      Author: IanB
 */
#include <avr/wdt.h>
#include <stdio.h>
#include "vectdata.h"
#include "usart_int.h"
#include "inputs.h"
#include "outputs.h"
#include "registers.h"
#include "hardware.h"
#include "buzzer.h"
#include "asciidef.h"
#include "sci.h"
#include "database.h"



/** USARTC connected to J9 (external RS232) */
ALLOCATE_USART_INT(usart2, USART_RX_BUFSIZE, USART_TX_BUFSIZE);

/** USARTD connected to J5 (R300 RS232) */
ALLOCATE_USART_INT(usart3, USART_RX_BUFSIZE, USART_TX_BUFSIZE);

/** USARTE connected to Bluetooth aircable */
ALLOCATE_USART_INT(usart1, USART_RX_BUFSIZE, USART_TX_BUFSIZE);


FILE usart2_stream;
FILE usart3_stream;
FILE usart1_stream;

int fdev_putchar(char ch, FILE *stream)
{
  int result;
  struct s_usart *usart;

  usart = (struct s_usart *) fdev_get_udata(stream);
  if (usart != NULL)
    result = usart_int_putchar(usart, ch);
  else
    result = -1;
  return result;
}


int fdev_getchar(FILE *stream)
{
  int result = -1;
  struct s_usart *usart;
  uint8_t ch;

  usart = (struct s_usart *) fdev_get_udata(stream);
  if (usart != NULL)
  {
    while (!usart_int_getchar(usart, &ch))
      wdt_reset();
    result = ch;
  }

  return result;
}


/** Get the number of characters available in the stream */
int fgetbufferlevel(FILE *stream)
{
  struct s_usart *usart;

  usart = (struct s_usart *) fdev_get_udata(stream);
  return usart_int_getrxlevel(usart);
}


/** Get if there is a entire message in the stream */
uint8_t fgetmessage(FILE *stream)
{
  struct s_usart *usart;
  uint8_t result = 0;
  uint8_t ch;

  usart = (struct s_usart *) fdev_get_udata(stream);
  // if we have data check if it is terminated
  if (usart_int_getrxlevel(usart) != 0)
  {
    usart_int_getlastchar(usart, &ch);
    if(!end_character_dc4(usart))
    {
    	switch (ch)
    	{
      	//case '\r':   // uncommit when message ends with \r character
      	case '\n':
      	case ';':
      		result = 1;
      		break;
    	}
    }
    else
    {
      switch (ch)
      {
        case H_DC4: // if DC4 detect end the stream
        	set_end_character_dc4(usart,false);
          result = 1;
          break;
      }
    }
  }
  return result;
}


/** Check if the buffer is full, flush if so */
void fcheckbufferfull(FILE *stream)
{
  struct s_usart *usart;

  usart = (struct s_usart *) fdev_get_udata(stream);
  if (usart_int_getrxfull(usart))
    usart_int_flushrx(usart);
}

/** Serial serial baud rate register setting for RS485 port*/ //(use serial baud rate index in database to access baud rate settings )
const uint32_t serialBaudRate[] = {2400, 4800 ,9600, 19200, 38400, 57600, 115200};

//** get serial baud rate from serial baud rate index */
uint32_t get_serial_baud_rate(uint8_t baudRateIndex)
{
	return serialBaudRate[baudRateIndex];
}

/**Check for serial baud rate available on the list*/
bool isSerialBaudAvailable(uint32_t tempBaud, uint8_t *index)
{
	bool status = false;
	uint8_t i = 0;

	for(i = 0; i < arrayLength(serialBaudRate); i++)
	{
		if(tempBaud == serialBaudRate[i])
		{
			index[0] = i;
			status = true;
			break;
		}
	}

	if(!status)
		index[0] = NUM_DB_BAUD_SETTINGS;

	return status;
}

/** Serial parity register settings*/
const char serialParity[] = {'n', 'e', 'o'};

//** get serial parity from serial parity index */
char get_serial_parity(uint8_t parityIndex)
{
	return serialParity[parityIndex];
}

/**Check for parity available on the list*/
bool isSerialParityAvailable(char tempParity, uint8_t *index)
{
	bool status = false;
	uint8_t i = 0;

	for(i = 0; i < arrayLength(serialParity); i++)
	{
		if((tempParity == serialParity[i]) || ((char)((uint8_t)tempParity + 32) == serialParity[i]))
		{
			index[0] = i;
			status = true;
			break;
		}
	}

	if(!status)
		index[0] = NUM_DB_PARITY_SETTINGS;

	return status;
}

/** Serial data bit register settings*/
const uint8_t serialDataBit[] = {5, 6, 7, 8};

//** get serial data bit from serial data bit index */
uint8_t get_serial_data_bit(uint8_t dataBitIndex)
{
	return serialDataBit[dataBitIndex];
}

/**Check for data bits available on the list*/
bool isSerialDataBitAvailable(char tempDataBit, uint8_t *index)
{
	bool status = false;
	uint8_t i = 0;

	for(i = 0; i < arrayLength(serialDataBit); i++)
	{
		if((uint8_t)(tempDataBit - '0') == serialDataBit[i])
		{
			index[0] = i;
			status = true;
			break;
		}
	}

	if(!status)
		index[0] = NUM_DB_DATA_SETTINGS;

	return status;
}

/** Serial stop bit register settings*/
const uint8_t serialStopBit[] = {1, 2};

//** get serial stop bit from serial stop bit index */
uint8_t get_serial_stop_bit(uint8_t dataBitIndex)
{
	return serialStopBit[dataBitIndex];
}

/**Check for stop bits available on the list*/
bool isSerialStopBitAvailable(char tempStopBit, uint8_t *index)
{
	bool status = false;
	uint8_t i = 0;

	for(i = 0; i < arrayLength(serialStopBit); i++)
	{
		if((uint8_t)(tempStopBit - '0') == serialStopBit[i])
		{
			index[0] = i;
			status = true;
			break;
		}
	}

	if(!status)
		index[0] = NUM_DB_STOP_SETTINGS;

	return status;
}

/** initialize usart hardware */
void init_usart(void)
{

  /* Initialise serial port structures */
  USART_INT_INIT(usart2, &USARTC0);
  USART_INT_INIT(usart3, &USARTD0);
  USART_INT_INIT(usart1, &USARTE0);

  /* Configure serial port hardware */
  // usart3 : port connected with main indicator (R320)
  // usart2 : external RS232 port
  // usart1 : Blue tooth port
  usart_int_internal_configure(&usart3,  51200, 'n', 8, 1); // this is to configure special baud rates (51200, 102400, 153600)
  //usart_int_configure(&usart3,  38400, 'n', 8, 1); // this is for normal testing

  // port which can configure through external commands
  usart_int_configure(&usart2, get_serial_baud_rate(userDBs.serialBaudIndex),
  															get_serial_parity(userDBs.serialParity),
																get_serial_data_bit(userDBs.serialData),
																get_serial_stop_bit(userDBs.serialStop));
  usart_int_configure(&usart1,  9600, 'n', 8, 1);

  /* Link streams to serial ports */
  fdev_setup_stream(&usart2_stream, fdev_putchar, fdev_getchar, _FDEV_SETUP_RW);
  fdev_set_udata(&usart2_stream, &usart2);
  fdev_setup_stream(&usart3_stream, fdev_putchar, fdev_getchar, _FDEV_SETUP_RW);
  fdev_set_udata(&usart3_stream, &usart3);
  fdev_setup_stream(&usart1_stream, fdev_putchar, fdev_getchar, _FDEV_SETUP_RW);
  fdev_set_udata(&usart1_stream, &usart1);
}



/** Initialise the hardware */
void init_hardware(void)
{
  PMIC.CTRL = PMIC_HILVLEX_bm | PMIC_MEDLVLEX_bm | PMIC_LOLVLEX_bm;

  init_usart();
  init_sci_stream();
  init_sc_buffers();
  inputs_init();
  outputs_init();
  InitBuzzer();
  init_read_port_A();
}

void *const vector_data[NUM_VECTORS] PROGMEM =
{
  NULL,         /*!< 0 - Reset vector */
  NULL,         /*!< 1 - External oscillator failure */
  NULL,         /*!< 2 - Port C interrupt 0 */
  NULL,         /*!< 3 - Port C interrupt 1 */
  NULL,         /*!< 4 - Port R interrupt 0 */
  NULL,         /*!< 5 - Port R interrupt 1 */
  NULL,         /*!< 6 - unused */
  NULL,         /*!< 7 - unused */
  NULL,         /*!< 8 - unused */
  NULL,         /*!< 9 - unused */
  NULL,         /*!< 10 - RTC overflow interrupt */
  NULL,         /*!< 11 - RTC compare interrupt */
  NULL,         /*!< 12 - TWI slave interrupt */
  NULL,         /*!< 13 - TWI master interrupt */
  NULL,         /*!< 14 - TCC0 overflow interrupt */
  NULL,         /*!< 15 - TCC0 error interrupt */
  NULL,         /*!< 16 - TCC0 CCA interrupt */
  NULL,         /*!< 17 - TCC0 CCB interrupt */
  NULL,         /*!< 18 - TCC0 CCC interrupt */
  NULL,         /*!< 19 - TCC0 CCD interrupt */
  NULL,         /*!< 20 - TCC1 overflow interrupt */
  NULL,         /*!< 21 - TCC1 error interrupt */
  NULL,         /*!< 22 - TCC1 CCA interrupt */
  NULL,         /*!< 23 - TCC1 CCB interrupt */
  NULL,         /*!< 24 - SPI interrupt */
  &usart2,      /*!< 25 - USARTC0 receive interrupt */
  &usart2,      /*!< 26 - USARTC0 data register empty interrupt */
  &usart2,      /*!< 27 - USARTC0 transmit complete interrupt */
  NULL,         /*!< 28 - unused */
  NULL,         /*!< 29 - unused */
  NULL,         /*!< 30 - unused */
  NULL,         /*!< 31 - unused */
  NULL,         /*!< 32 - NVM EE interrupt */
  NULL,         /*!< 33 - NVM SPM interrupt */
  NULL,         /*!< 34 - Port B interrupt 0 */
  NULL,         /*!< 35 - Port B interrupt 1 */
  NULL,         /*!< 36 - unused */
  NULL,         /*!< 37 - unused */
  NULL,         /*!< 38 - unused */
  NULL,         /*!< 39 - unused */
  NULL,         /*!< 40 - unused */
  NULL,         /*!< 41 - unused */
  NULL,         /*!< 42 - unused */
  NULL,         /*!< 43 - Port E interrupt 0 */
  NULL,         /*!< 44 - Port E interrupt 1 */
  NULL,         /*!< 45 - TWI slave interrupt */
  NULL,         /*!< 46 - TWI master interrupt */
  NULL,         /*!< 47 - TCE0 overflow interrupt */
  NULL,         /*!< 48 - TCE0 error interrupt */
  NULL,         /*!< 49 - TCE0 CCA interrupt */
  NULL,         /*!< 50 - TCE0 CCB interrupt */
  NULL,         /*!< 51 - TCE0 CCC interrupt */
  NULL,         /*!< 52 - TCE0 CCD interrupt */
  NULL,         /*!< 53 - unused */
  NULL,         /*!< 54 - unused */
  NULL,         /*!< 55 - unused */
  NULL,         /*!< 56 - unused */
  NULL,         /*!< 57 - unused */
  &usart1,      /*!< 58 - USARTE0 receive interrupt */
  &usart1,      /*!< 59 - USARTE0 data register empty interrupt */
  &usart1,      /*!< 60 - USARTE0 transmit complete interrupt */
  NULL,         /*!< 61 - unused */
  NULL,         /*!< 62 - unused */
  NULL,         /*!< 63 - unused */
  NULL,         /*!< 64 - Port D interrupt 0 */
  NULL,         /*!< 65 - Port D interrupt 1 */
  &usart2_stream,  /*!< 66 - Port A interrupt 0 */
  NULL,         /*!< 67 - Port A interrupt 1 */
  NULL,         /*!< 68 - AC0 interrupt */
  NULL,         /*!< 69 - AC1 interrupt */
  NULL,         /*!< 70 - ACW window interrupt */
  NULL,         /*!< 71 - ADCA interrupt 0 */
  NULL,         /*!< 72 - unused */
  NULL,         /*!< 73 - unused */
  NULL,         /*!< 74 - unused */
  NULL,         /*!< 75 - unused */
  NULL,         /*!< 76 - unused */
  NULL,         /*!< 77 - TCD0 overflow interrupt */
  NULL,         /*!< 78 - TCD0 error interrupt */
  NULL,         /*!< 79 - TCD0 CCA interrupt */
  NULL,         /*!< 80 - TCD0 CCB interrupt */
  NULL,         /*!< 81 - TCD0 CCC interrupt */
  NULL,         /*!< 82 - TCD0 CCD interrupt */
  NULL,         /*!< 83 - unused */
  NULL,         /*!< 84 - unused */
  NULL,         /*!< 85 - unused */
  NULL,         /*!< 86 - unused */
  NULL,         /*!< 87 - SPI interrupt */
  &usart3,      /*!< 88 - USARTD0 receive interrupt */
  &usart3,      /*!< 89 - USARTD0 data register empty interrupt */
  &usart3,      /*!< 90 - USARTD0 transmit complete interrupt */
  NULL,         /*!< 91 - unused */
  NULL,         /*!< 92 - unused */
  NULL,         /*!< 93 - unused */
  NULL,         /*!< 94 - unused */
  NULL,         /*!< 95 - unused */
  NULL,         /*!< 96 - unused */
  NULL,         /*!< 97 - unused */
  NULL,         /*!< 98 - unused */
  NULL,         /*!< 99 - unused */
  NULL,         /*!< 100 - unused */
  NULL,         /*!< 101 - unused */
  NULL,         /*!< 102 - unused */
  NULL,         /*!< 103 - unused */
  NULL,         /*!< 104 - Port F interrupt 0 */
  NULL,         /*!< 105 - Port F interrupt 1 */
  NULL,         /*!< 106 - unused */
  NULL,         /*!< 107 - unused */
  NULL,         /*!< 108 - TCF0 overflow interrupt */
  NULL,         /*!< 109 - TCF0 error interrupt */
  NULL,         /*!< 110 - TCF0 CCA interrupt */
  NULL,         /*!< 111 - TCF0 CCB interrupt */
  NULL,         /*!< 112 - TCF0 CCC interrupt */
  NULL,         /*!< 113 - TCF0 CCD interrupt */
};
