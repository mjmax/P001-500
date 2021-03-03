/*
 * sci.c
 *
 *  Created on: 05/03/2019
 *      Author: Janaka Madhusankha
 */
#include <avr/io.h>
#include "sci.h"
#include "hardware.h"
#include "usart_int.h"
#include "string.h"
#include "asciidef.h"
#include "command.h"
#include "_sprintf.h"

// sci port array (do not change elements)
FILE sci_strm[MAX_SCI_PORT]; //= {usart1_stream,usart2_stream,usart3_stream};
char message[BUFFER_SIZE];
sc_buffer specialCH;

void init_sc_buffers(void)
{
	init_sc_buffer(&specialCH);
	// add more buffers if needed
}

void init_sc_buffer(sc_buffer *buffer)
{
	memset(buffer->SCBuffer, '\0', arlen(buffer->SCBuffer));
	buffer->pointer = 0;
	buffer->free = arlen(buffer->SCBuffer);
}

bool put_char_sc_buffer(sc_buffer *buff, uint8_t ch) // set character to buffer return false if buffer overflow
{
	bool status = false;
  uint8_t sReg;

  /* disable interrupts globally */
  sReg = SREG;
  SREG = 0;
	if(buff->free > 0)
	{
		buff->SCBuffer[buff->pointer] = ch;
		buff->pointer++;
		buff->free--;
		status = true;
	}
	else
		status = false;

	SREG = sReg;
	return status;
}

uint8_t get_last_char_sc_buffer(sc_buffer *buff) // return last character of the buffer, return NULL if buffer empty
{
	uint8_t ch;
  uint8_t sReg;

  /* disable interrupts globally */
  sReg = SREG;
  SREG = 0;
	if(buff->pointer == 0)
		ch = 0x00;
	else
	{
		ch = buff->SCBuffer[buff->pointer - 1];
		buff->pointer--;
		buff->free++;
	}

	SREG = sReg;
	return ch;
}

uint8_t get_any_char_sc_buffer(sc_buffer *buff, uint8_t pointer) // return any character without erasing buffer null if item not found
{
	uint8_t ch;
  uint8_t sReg;

  /* disable interrupts globally */
  sReg = SREG;
  SREG = 0;
	if(pointer >= buff->pointer)
	{
		ch = 0x00;
	}
	else
		ch = buff->SCBuffer[pointer];
	SREG = sReg;
	return ch;
}

uint8_t get_sc_buffer_level(sc_buffer *buff)
{
  uint8_t sReg;
  uint8_t bufLevel;

  /* disable interrupts globally */
  sReg = SREG;
  SREG = 0;
  bufLevel = buff->pointer;
  SREG = sReg;
	return bufLevel;
}


void get_rx_buffer(eSciPort sci, char *msg)
{
	int count = 0;
	char message[USART_RX_BUFSIZE];
	while((fgetbufferlevel(&sci_strm[sci]) > 0) && (count < USART_RX_BUFSIZE))
	{
		message[count] = (char )fdev_getchar(&sci_strm[sci]);
		count++;
	}
	message[count++] = '\0';
	strcpy(msg, message);
}

void sci_set_current_port(eSciPort sci)
{
	appVariables.currentSciPort = sci;
}

eSciPort sci_get_current_port(void)
{
	return	appVariables.currentSciPort;
}

bool sci_get_new_message(eSciPort sci)
{
	return ((fgetmessage(&sci_strm[sci]) > 0))? true:false;
}

// flush relevant port buffer if full
void sci_rx_clear(eSciPort sci)
{
	fcheckbufferfull(&sci_strm[sci]);
}

void init_sci_stream(void)
{
	sci_strm[SCI_PORT_BT] = usart1_stream;
	sci_strm[SCI_PORT_RS232] = usart2_stream;
	sci_strm[SCI_PORT_INTERNAL] = usart3_stream;
}

void PutCharactor(char ch, FILE *stream)
{
	while(!(fdev_putchar((char) ch , stream))){}
}

void LoadMessage(eSciPort port)
{
	FILE *stream;
	uint8_t count = 0;

	stream = get_stream(port);
	memset(message,'\0',BUFFER_SIZE);
	while(fgetbufferlevel(stream) > 0)
	{
		message[count++] = (char)fdev_getchar(stream);
		if ((count > BUFFER_SIZE - 3))
			break;
	}
}

void CommsSendString(char *reply, eSciPort port)
{
	uint8_t count =0;

	while(*reply != '\0')
	{
		PutCharactor((char) *reply++, get_stream(port));
		if(count>REPLY_BUFFER_LENGTH)
			break;
		count++;
	}
}

FILE *get_stream(eSciPort sci)
{
	return &sci_strm[sci];
}

// output characters with encoded and type of encoded
// return 0 if nothing found

bool checkForSpecialCharacters(char *ch)
{
	byte i = 0;
	byte status = false;

	if(get_sc_buffer_level(&specialCH) > 0) // pull all characters from buffer to pointer
	{
		while (get_sc_buffer_level(&specialCH) > 0)
			ch[i++] = (char)get_last_char_sc_buffer(&specialCH); // load character LIFO basis
		ch[i++] = '\0';
		i = 0;
		status = true;
	}
	return status;
}

byte GetSpecialChatacterType(char ch)
{
	byte type = CHAR_ENCODED_IO;
	switch(ch & SPECIAL_CHAR_TYPE_MASK)
	{
		case SPECIAL_CHAR_TYPE_IO:
			type = CHAR_ENCODED_IO;
			break;
		case SPECIAL_CHAR_TYPE_DAC:
			type = CHAR_ENCODED_DAC;
			break;
	}
	return type;
}

void SendDac(uint16_t dacValue,byte type)
{
	char test[20];
	char numberString[10];
	static byte previousType = DAC_TYPE_VOLTAGE;

	memset(numberString, '\0', arlen(numberString));
	UnsignedShortToString(dacValue ,numberString ,10);
	switch(type)   //// <DLE> = "\x10", PORT_CMD = "0"
	{
	case DAC_TYPE_VOLTAGE:
		_sprintf(test,"%c%cDAC:%s,V\r\n", H_DLE, PORT_CMD, numberString);
		previousType = type;
		break;
	case DAC_TYPE_CURRENT:
		_sprintf(test,"%c%cDAC:%s,I\r\n", H_DLE, PORT_CMD, numberString);
		previousType = type;
		break;
	case DAC_TYPE_MAX:
		switch(previousType)
		{
			case DAC_TYPE_VOLTAGE:
				_sprintf(test,"%c%cDAC:%s,V\r\n", H_DLE, PORT_CMD, numberString);
				previousType = DAC_TYPE_VOLTAGE;
				break;
			case DAC_TYPE_CURRENT:
				_sprintf(test,"%c%cDAC:%s,I\r\n", H_DLE, PORT_CMD, numberString);
				previousType = DAC_TYPE_CURRENT;
				break;
		}
		break;
	}
	CommsSendString(test,SCI_DEFAULT_PORT);
}

void extractDACValue(char ch)
{
	byte type;
	static uint8_t bit24[3] = {SPECIAL_CHAR_TYPE_DAC,SPECIAL_CHAR_TYPE_DAC,SPECIAL_CHAR_TYPE_DAC};
	static uint8_t pointer = 0;
	uint16_t dacValue;

	bit24[pointer] = (uint8_t)(ch & 0X3F); // left the upper two bits (make upper two bit as zero)

	if(pointer == 2)
	{
		dacValue = (uint16_t)(bit24[0] << 10) | (uint16_t)(bit24[1] << 4) | (uint16_t)(bit24[2] >> 2); // construct a 16bit value
		switch(bit24[2] & SPECIAL_CHAR_DAC_TYPE_MASK)
		{
			case SPECIAL_CHAR_DAC_TYPE_VOLTAGE:
				type = DAC_TYPE_VOLTAGE;
				break;
			case SPECIAL_CHAR_DAC_TYPE_CURRENT:
				type = DAC_TYPE_CURRENT;
				break;
			default:
				type = DAC_TYPE_MAX;
				break;
		}
		SendDac(dacValue, type);
		memset(bit24, '\0', arlen(bit24));
		pointer = 0;
	}
	else
		pointer += 1;

}

void extractOutputValue(char ch)
{
	PORTB.OUT =  (uint8_t)(ch & 0X0F); // filter last four bits to get four outputs and set the bits
}

void DecodeSpecilaCharacters(char ch)
{
	byte type;

	type = GetSpecialChatacterType(ch);
	switch(type)
	{
		case CHAR_ENCODED_IO:
			extractOutputValue(ch);
			break;
		case CHAR_ENCODED_DAC:
			extractDACValue(ch);
			break;
			// add more cases as necessary
	}
}

void checkEncodedChar(void)
{
	char ch[SPECILA_CHAR_BUFFER_SIZE];
	bool scAvailable = false;
	byte count = 0;

	memset(ch, '\0', arlen(ch));
	scAvailable = checkForSpecialCharacters(ch);

	if(scAvailable)
	{
		while(ch[count] != '\0')
			DecodeSpecilaCharacters((char) ch[count++]);
	}
}

void RinNetMsgProcess(void)
{
	char tempData[BUFFER_SIZE];
	int count = 0;

	if(message[1] == (char)H_DLE)	// if DLE then re arrange the string (skip DLE and Port characters)
	{
		tempData[count] = message[count];
		for (count = 3; count + 1 >= strlen(message); count++)
		{
			tempData[count] = message[count];
		}
		CommsSendString(tempData,SCI_DEFAULT_PORT);
	}
	else
		CommsSendString(message,SCI_DEFAULT_PORT);
}

void DefaultMsgProcess(void)
{
	char commandToExecute[MAX_MODULE_COMM_BUFFER];
	char tempMessage[5][BUFFER_SIZE]; // 5 is maximum numbers of messages contain in one buffer lenth
	int count = 0;
	int msgCount = 0;
	int poniCount = 0;
	bool allMessageProcessed = true;

	memset(tempMessage,'\0',sizeof(tempMessage));

	for(count = 0; count <= strlen(message); count++) // search for DLEs
	{
		tempMessage[msgCount][poniCount++] = message[count];
		if(message[count+1] == (char)H_DLE)
		{
			msgCount += 1; // more than one message
			poniCount = 0;
		}
	}

	while (allMessageProcessed)
	{
		switch (tempMessage[msgCount][1])	// multiplex the message according to the port
		{
			case (char)PORT_CMD:
				memset(commandToExecute,'\0',arlen(commandToExecute));
				strncpy(commandToExecute,&tempMessage[msgCount][2],strlen(&tempMessage[msgCount][2]));
				CommandHandle(commandToExecute);
				//CommsSendString(commandToExecute,SCI_PORT_RS232); // test
				break;
			case (char)PORT_BT:
				CommsSendString(&tempMessage[msgCount][2],SCI_PORT_BT);
				break;
			case (char)PORT_EXT:
				CommsSendString(&tempMessage[msgCount][2],SCI_PORT_RS232); // remove first two characters
				break;
		}
		if(msgCount <= 0)
			allMessageProcessed = false;

		msgCount -= 1;
	}
}

void multiplex_internal_port(void)
{
	eSciPort internalStream = SCI_PORT_INTERNAL;
	sci_set_current_port(internalStream);
	LoadMessage(internalStream);
	switch(message[0])
	{
		case (char)H_DC2:
			RinNetMsgProcess();
			break;
		case (char)H_DLE:
			DefaultMsgProcess();
			//PutCharactor((char) 0x41 ,get_stream(SCI_PORT_RS232));
			break;
		default:
			CommsSendString(message,SCI_DEFAULT_PORT); // send all data out from internal generation to rear port
			break;
	}
}

char increaseAddress(char currentAddress)
{
	byte moduleNum = EX_IO_MODULE_1;
	char addID[] = {'9', 'A', 'B', 'C', 'D', 'E', 'F'};
	char nextAddress = currentAddress;

	for(moduleNum = EX_IO_MODULE_1 ; moduleNum < (EX_IO_MODULE_MAX - 1) ;  moduleNum++)
	{
		if(addID[moduleNum] == currentAddress)
		{
			nextAddress = addID[moduleNum + 1];
			break;
		}
	}
	return nextAddress;
}

void multiplex_rs232_port(void)
{
	char tempMessage[BUFFER_SIZE], nextAdd;
	byte addressPoint = 2;
	eSciPort rs232Stream = SCI_PORT_RS232;
	eSciPort targerStream = SCI_PORT_INTERNAL;

	sci_set_current_port(rs232Stream);
	LoadMessage(rs232Stream);
	memset(tempMessage, '\0', arlen(tempMessage));
	if(message[0] == (char) H_DC2)
		CommsSendString(message,targerStream);
	else if(message[0] == (char) H_DLE)
	{
		nextAdd = increaseAddress(message[addressPoint]);
		message[addressPoint] = nextAdd;
		CommsSendString(message,targerStream);
	}
	else
	{
		_sprintf(tempMessage,"%c%c%s",H_DLE,PORT_EXT,message);
		CommsSendString(tempMessage,targerStream);
	}
}

void multiplex_bt_port(void)
{
	char tempMessage[BUFFER_SIZE];
	eSciPort btStream = SCI_PORT_BT;
	eSciPort targerStream = SCI_PORT_INTERNAL;

	sci_set_current_port(btStream);
	LoadMessage(btStream);
	memset(tempMessage,'\0',sizeof(tempMessage));
	_sprintf(tempMessage,"%c%c%s",H_DLE,PORT_BT,message);
	CommsSendString(tempMessage,targerStream);
}

void MultiplexSciPort(eSciPort sci)
{
	switch (sci)
	{
	case SCI_PORT_INTERNAL:
		multiplex_internal_port();
		break;
	case SCI_PORT_RS232:
		multiplex_rs232_port();
		break;
	case SCI_PORT_BT:
		multiplex_bt_port();
		break;
	case MAX_SCI_PORT:
		break;
	}
}
