/*
 * command.h
 *
 *  Created on: 27/04/2018
 *      Author: Janaka Madhusankha
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "registers.h"
#include "strings.h"
#include "asciidef.h"
#include "inputs.h"
#include "outputs.h"
#include "hardware.h"
#include "command.h"
#include "buzzer.h"
#include "_sprintf.h"
#include "database.h"
#include "eeprom.h"

bool configMode = false;

static const struct rinReg_t rinRegList[] =
{
		{MODEL_REG,												RR_,		ModelNumber},
		{STATUS_REG,											RR_,		CurrentStatus},
		{CONFIG_TYPE_REG, 								R_W,		ConfigType},
		{CONFIG_DATA_REG, 								RRW,		ConfigData},
		{SOFTWARE_VERSION_REG, 						RR_,		SoftwareVersion},

		{SERIAL_NO_REG, 									RRW,		SerialNumber},

		//{USART_EXTERNAL_REG, 							RRW,		SerialBaudRate},
		{BUZZER_STATUS_REG,								RRW,		BuzzerStatus},
		{OUTPUT_LED_STATUS_REG,						RRW,		OutputLedStatus},

		{INPUT_STATUS_REG, 								RR_,		InputStatus},
		{OUTPUT_STATUS_REG, 							RRW,		OutputStatus},
		{ANALOGE_OUTPUT_REG, 							RRW,		AnalogeOutput},


		{ END_REG, 												RRW,		NullTest}                     // identifies the end of the command array
};


static const struct rinReg_t configRegList[] =
{
		{CANCEL_CONFIG_MODE, 							R_W,		CancelConfigMode},
		{CONFIG_WRITE_REPLY_ACK, 					RRW,		WriteReplyAck},
		{CONFIG_USART_EXTERNAL, 					RRW,		SerialExternalBaud},
		//{CONFIG_USART_INTERNAL, 					RRW,		SerialInternalBaud},
		{CONFIG_IO_DIRECTIONS, 						RRW,		IODirection},

		{ END_REG, 												RRW,		NullTest}                     // identifies the end of the command array

};

bool IsConfigMode(void)
{
	return configMode;
}

void SetConfigMode(bool staus)
{
	configMode = staus;
}

/** Lookup Reg Mode and return true if the incoming mode from rinCommand allow.*/
bool LookupRegMode(byte regMode)
{
	bool status = false;
	byte incomingRegMode;

	incomingRegMode = GetIncomingRegMode();
	switch (incomingRegMode)
	{
	case REG_READABLE:
		status = ((regMode == RRW) || (regMode == RR_))? true: false;
		break;
	case REG_WRITABLE:
		status = ((regMode == RRW) || (regMode == R_W))? true: false;
		break;
	}

	return status;
}

/** Lookup cmd in the list of tests, and run the command.
 * Returns magic 20 if not found.
 * Returns magic 21 if register is not accessible with the current mode
 * */
static char LookupAndRunCommand(byte reg, const struct rinReg_t test[], char *param_ptr)
{
  int i = 0;
  char result;
  byte regMode = 0;

  /* Locate command */
  while(!((byte)test[i].reg == reg))
  {
  	if((test[i].reg == END_REG))
  		break;
  	i += 1;
  }


  if(test[i].reg == END_REG)
  	result = test[i].testfn(regMode, param_ptr);
  else
  {
  	if(LookupRegMode(test[i].mode))
  	{
  		switch(GetIncomingRegMode())
  		{
  			case REG_READABLE:
  				regMode = REG_MODE_READ;
  				break;
  			case REG_WRITABLE:
  				regMode = REG_MODE_WRITE;
  				break;
  		}
  		result = test[i].testfn(regMode, param_ptr);
  	}
  	else
  		result = 21;
  }

  return result;
}


void GetRegValue(byte addres ,char *data)
{
	byte res;

	if(IsConfigMode())
	{
		if(addres == CONFIG_DATA_REG)
			res = LookupAndRunCommand(GetConfigAddress() ,configRegList ,data);
		else
		{
			SetConfigMode(false);
			res = LookupAndRunCommand(addres ,rinRegList ,data);
		}
	}
	else
		res = LookupAndRunCommand(addres ,rinRegList ,data);

	switch (res)		// see magic 20 or 21 (20 is responsible for no reg found, 21 is responsible for authentication fails)
	{
		case (char)23:
			_sprintf(data, "%s\r\n", WRITE_FAIL_STRING);
			break;
		case (char)22:
			_sprintf(data, "%s\r\n", WRITE_SUCCESS_STRING);
			break;
		case (char)21:
			_sprintf(data, "%s\r\n", AUTHENTICATION_FAIL_STRING);
			break;
		case (char)20:
			_sprintf(data, "%s\r\n", INVALID_RIGISTER_STRING);
			break;
	}
}

/*...............................Register Related functions....................................
 * PROTOTYPE: char FunctionName (eRegMode mode, char *str);
 * 		output type : (char) contain status of the function executed
 * 		input type  : a. (eRegMode) should provide whether read, write or execute
 * 									b. (char *)		contain output result if any
 *
 * Function body consist of 3 main parts according to the "mode" separated by switch statement
 * leave switch case as null in side case body if nothing do do with particular mode
 *
 * read the existion function body to get more detail
 * */


char SerialNumber(eRegMode mode, char *str)
{
	char szTemp[25];
	char *nonConvert;
	char status = 0;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		my_ultoa(factoryDBs.serialNo, szTemp, 10);
		_sprintf(str, "%s\r\n", szTemp);
		status = 1;
		break;
	case REG_MODE_WRITE:
		factoryDBs.serialNo = (uint32_t)strtoul(GetRegDataPointer(),&nonConvert,10);
		WriteFactoryDB();
		status = 22;	// magic 22 is responsible for write operation successful
		break;
	}

	return status;
}

char CurrentStatus(eRegMode mode, char *str)
{
	char status = 0;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
//		_sprintf(str, "%s\r\n", SOFTWARE_VERSION_STRING);
//		status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

char ConfigType(eRegMode mode, char *str)
{
	char status = 0;
	char *nonConvert;
	byte tempConfigueAdd;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		// nothing to read
		break;
	case REG_MODE_WRITE:
		tempConfigueAdd = (byte)strtol(GetRegDataPointer(),&nonConvert,10);	// extract config data address
		SetConfigAddress(tempConfigueAdd);																	// keep the address for next message
		SetConfigMode(true);																								// turn on configuration mode
		status = 22;	// magic 22 is responsible for write operation successful
		break;
	}

	return status;
}

char ConfigData(eRegMode mode, char *str)
{
	char status = 0;
/*
 * this is a special register function.
 * there is no any direct value to read or write this register.
 * the value is redirected to the config register list functions.
 * so when it is writing or reading value from this register,
 * it automatically read and write the values related to its own register setup by the previous message
 *
 */
	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		// nothing to Write
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

char SoftwareVersion(eRegMode mode, char *str)
{
	char status = 0;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		_sprintf(str, "%s\r\n", SOFTWARE_VERSION_STRING);
		status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

//char RegVersion(eRegMode mode, char *str)
//{
//	char status = 0;
//
//	switch (mode)
//	{
//	case REG_MODE_EXECUTE:
//		// nothing to execute
//		break;
//	case REG_MODE_READ:
//		_sprintf(str, "%s\r\n", REG_VERSION_STRING);
//		status = 1;
//		break;
//	case REG_MODE_WRITE:
//		// nothing to Write
//		break;
//	}
//
//	return status;
//}

char CopyrightString(eRegMode mode, char *str)
{
	char status = 0;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		_sprintf(str, "%s\r\n", COPYRIGHT_STRING);
		status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

char ModelNumber(eRegMode mode, char *str)
{
	char status = 0;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		_sprintf(str, "%s\r\n", MODEL_STRING);
		status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

static const char *ledMode[] = {"OFF", "OVER", "UNDER", "PASS"};

char OutputLedStatus(eRegMode mode, char *str)
{
	char status = 0;
	char *nonConvert;
	uint8_t tempLed;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		_sprintf(str, "%s\r\n", ledMode[appVariables.status.ledOutStatus]);  // filter out first four inputs (0x0F)
		status = 1;
		break;
	case REG_MODE_WRITE:
		tempLed = (uint8_t)strtol(GetRegDataPointer(),&nonConvert,10);
		if((tempLed >= LED_OUT_OFF) && (tempLed < MAX_LED_STATUS)) // if the output LED status is within the range
		{
			if(appVariables.status.ledOutStatus != tempLed) // if the value is changed only
			{
				appVariables.status.ledOutStatus = tempLed;
				PORTB.OUTCLR = 0x70; // clear all LED outputs (bit 7,6,5)
				PORTB.OUTSET = 0x70 & (1 << (appVariables.status.ledOutStatus + 3)); // portB bit 7,6,5 are the LED pin. (0x70 is to filter those bits and +3 is to shift the output bit up to fourth bit)
				status = 22; // write operation success
			}
		}
		else
			status = 23; // write fail string
		break;
	}

	return status;
}

char InputStatus(eRegMode mode, char *str)
{
	char status = 0;
	uint8_t inpMask = (uint8_t)(((~userDBs.ioDirection) >> 4) & POART_A_MAX_INP);

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		appVariables.status.inputStatus = (uint8_t)(PORTA.IN & inpMask);
		_sprintf(str,"%d\r\n",appVariables.status.inputStatus);
		status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

char OutputStatus(eRegMode mode, char *str)
{
	char status = 0;
	char *nonConvert;
	uint8_t tempOut;
	uint8_t cdOutputFilter = (uint8_t)((1 << (COMMS_DRIVEN_OUTPUT)) - 1);		// comms driven outputs;
	uint8_t outputFilter = (userDBs.ioDirection | FIXED_OUTPUT_MASK);//(uint8_t)((1 << (NUMBER_OF_OUTPUT)) - 1);				// all outputs;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		// read all outputs (comms driven + direct driven)
		_sprintf(str,"%d\r\n",(appVariables.status.outputStatus & outputFilter));
		status = 1;
		break;
	case REG_MODE_WRITE:
		tempOut = (uint8_t)strtol(GetRegDataPointer(),&nonConvert,10);
		if(tempOut >= 0)
		{
			if(appVariables.status.outputStatus != tempOut)
			{
				appVariables.status.outputStatus = tempOut;
				PORTB.OUTCLR = cdOutputFilter; // clear comms driven outputs
				PORTB.OUTSET = cdOutputFilter & (appVariables.status.outputStatus); // set comms driven output
			}
			status = 22; // write operation success
		}
		else
			status = 23; // write fail string
		break;
	}

	return status;
}

char AnalogeOutput(eRegMode mode, char *str)
{
	char status = 0;
	char szTemp[25];

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		UnsignedShortToString(appVariables.status.analogOutStatus ,szTemp ,10);
		_sprintf(str, "%s\r\n", szTemp);
		status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

//char SerialBaudRate(eRegMode mode, char *str)
//{
//	char status = 0;
//	char szTemp[25];
//	char *nonConvert;
//	char tempData[10];
//	uint8_t tempParitydIndex;
//	uint8_t tempDataBitIndex;
//	uint8_t tempStopBitIndex;
//	uint8_t tempBaudIndex;
//	uint32_t tempBaud;
//	bool serialBitsOk = false;
//	bool serialBaudOk = false;
//
//	switch (mode)
//	{
//		case REG_MODE_EXECUTE:
//			// nothing to execute
//			break;
//		case REG_MODE_READ:
//			my_ultoa(get_serial_baud_rate(userDBs.serialBaudIndex), szTemp, 10); // convert baud rate in to string
//			_sprintf(str, "%s%c%d%d\r\n", szTemp, get_serial_parity(userDBs.serialParity),
//																						get_serial_data_bit(userDBs.serialData),
//																						get_serial_stop_bit(userDBs.serialStop)); // combine baud + serial bits
//			status = 1;
//			break;
//		case REG_MODE_WRITE:
//			tempBaud = (uint32_t)strtoul(GetRegDataPointer(),&nonConvert,10);
//			strcpy(tempData,nonConvert);
//			serialBitsOk = (isSerialParityAvailable(tempData[0], &tempParitydIndex) &&
//											isSerialDataBitAvailable(tempData[1], &tempDataBitIndex) &&
//											isSerialStopBitAvailable(tempData[2], &tempStopBitIndex))? true : false; // check serial bits available
//			serialBaudOk = (isSerialBaudAvailable(tempBaud, &tempBaudIndex))? true : false; // check serial baud available
//			if(serialBitsOk && serialBaudOk) // if both bits and baud are real
//			{
//				serialBitsOk = false; // reset for another purpose
//				serialBaudOk = false; // reset for another purpose
//
//				if(userDBs.serialBaudIndex != tempBaudIndex) // write if the baud is change only else just "Ok"
//				{
//					userDBs.serialBaudIndex = tempBaudIndex;
//					WriteUserDB();
//					ReadUserDB();
//					serialBaudOk = true;
//				}
//
//				if(userDBs.serialParity != tempParitydIndex ||	userDBs.serialData != tempDataBitIndex || userDBs.serialStop != tempStopBitIndex)  // write if the data bits are change only else just "Ok"
//				{
//					userDBs.serialParity = tempParitydIndex;
//					userDBs.serialData = tempDataBitIndex;
//					userDBs.serialStop = tempStopBitIndex;
//					WriteUserDB();
//					ReadUserDB();
//					serialBitsOk = true;
//				}
//
//				if(serialBaudOk || serialBitsOk) // if serial settings were changed then restart program
//				{
//					my_ultoa(get_serial_baud_rate(userDBs.serialBaudIndex), szTemp, 10);
//					_sprintf(str, "new_serial_settings:%s%c%d%d\r\n", szTemp, get_serial_parity(userDBs.serialParity),
//																												 	 	 	 	 	  get_serial_data_bit(userDBs.serialData),
//																																	  get_serial_stop_bit(userDBs.serialStop));
//					CommsSendString(str,sci_get_current_port());
//					while(true); // restart the program. nothing valid this point below(wait until watch dog reset the program)
//					status = 22;		// magic 22 is responsible for write operation successful
//				}
//				status = 22;		// magic 22 is responsible for write operation successful
//			}
//			else
//				status = 23;		// magic 23 is responsible for write operation fail
//
//			break;
//	}
//
//	return status;
//}

char BuzzerStatus(eRegMode mode, char *str)
{
	char status = 0;
	char *nonConvert;
	uint8_t tempBuzzer;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		// nothing to read
		break;
	case REG_MODE_WRITE:
		tempBuzzer = (uint8_t)strtoul(GetRegDataPointer(),&nonConvert,10);
		if((tempBuzzer >= BUZZER_OFF) && (tempBuzzer < BUZZER_BEEP_MAX))
		{
			appVariables.status.buzzerStatus = tempBuzzer;
			setBuzzer(appVariables.status.buzzerStatus);
		}
		else
			status = 23;		// magic 23 is responsible for write operation fail

		break;
	}

	return status;
}

char NullTest(eRegMode mode, char *str)
{
	if(IsConfigMode())
		SetConfigMode(false);		 // turn off configuration mode irrespective of mode

	return 20;
}



/*............................Configuration type Related functions..............................
 * PROTOTYPE: char FunctionName (eRegMode mode, char *str);
 * 		output type : (char) contain status of the function executed
 * 		input type  : a. (eRegMode) should provide whether read, write or execute
 * 									b. (char *)		contain output result if any
 *
 * Function body consist of 3 main parts according to the "mode" separated by switch statement
 * leave switch case as null in side case body if nothing do do with particular mode
 *
 * read the existion function body to get more detail
 * */

char CancelConfigMode(eRegMode mode, char *str)
{
	char status = 0;

	SetConfigMode(false);		 // turn off configuration mode irrespective of mode
	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		// status = 1;
		break;
	case REG_MODE_WRITE:
		// nothing to Write
		break;
	}

	return status;
}

char WriteReplyAck(eRegMode mode, char *str)
{
	char status = 0;
	char *nonConvert;
	uint8_t writeReplyAck;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		_sprintf(str,"%d\r\n",userDBs.writeReplyAck);
		SetConfigMode(false);		 	// turn off configuration mode irrespective of mode
		status = 1;
		break;
	case REG_MODE_WRITE:
		writeReplyAck = (uint8_t)strtol(GetRegDataPointer(),&nonConvert,10);
		if((writeReplyAck >= WRITE_ACK_DISABLE) && (writeReplyAck < MAX_WRITE_ACK))
		{
			if(userDBs.writeReplyAck != writeReplyAck)
			{
				userDBs.writeReplyAck = writeReplyAck;
				WriteUserDB();
				ReadUserDB();
			}
			SetConfigMode(false);		 	// turn off configuration mode irrespective of mode
			status = 22; 							// write operation success
		}
		else
			status = 23;		// magic 23 is responsible for write operation fail
		break;
	}

	return status;
}

char SerialExternalBaud(eRegMode mode, char *str)
{
	char status = 0;
	char szTemp[25];
	char *nonConvert;
	char tempData[10];
	uint8_t tempParitydIndex;
	uint8_t tempDataBitIndex;
	uint8_t tempStopBitIndex;
	uint8_t tempBaudIndex;
	uint32_t tempBaud;
	bool serialBitsOk = false;
	bool serialBaudOk = false;

	switch (mode)
	{
		case REG_MODE_EXECUTE:
			// nothing to execute
			break;
		case REG_MODE_READ:
			my_ultoa(get_serial_baud_rate(userDBs.serialBaudIndex), szTemp, 10); // convert baud rate in to string
			_sprintf(str, "%s%c%d%d\r\n", szTemp, get_serial_parity(userDBs.serialParity),
																						get_serial_data_bit(userDBs.serialData),
																						get_serial_stop_bit(userDBs.serialStop)); // combine baud + serial bits
			status = 1;
			break;
		case REG_MODE_WRITE:
			tempBaud = (uint32_t)strtoul(GetRegDataPointer(),&nonConvert,10);
			strcpy(tempData,nonConvert);
			serialBitsOk = (isSerialParityAvailable(tempData[0], &tempParitydIndex) &&
											isSerialDataBitAvailable(tempData[1], &tempDataBitIndex) &&
											isSerialStopBitAvailable(tempData[2], &tempStopBitIndex))? true : false; // check serial bits available
			serialBaudOk = (isSerialBaudAvailable(tempBaud, &tempBaudIndex))? true : false; // check serial baud available
			if(serialBitsOk && serialBaudOk) // if both bits and baud are real
			{
				serialBitsOk = false; // reset for another purpose
				serialBaudOk = false; // reset for another purpose

				if(userDBs.serialBaudIndex != tempBaudIndex) // write if the baud is change only else just "Ok"
				{
					userDBs.serialBaudIndex = tempBaudIndex;
					WriteUserDB();
					ReadUserDB();
					serialBaudOk = true;
				}

				if(userDBs.serialParity != tempParitydIndex ||	userDBs.serialData != tempDataBitIndex || userDBs.serialStop != tempStopBitIndex)  // write if the data bits are change only else just "Ok"
				{
					userDBs.serialParity = tempParitydIndex;
					userDBs.serialData = tempDataBitIndex;
					userDBs.serialStop = tempStopBitIndex;
					WriteUserDB();
					ReadUserDB();
					serialBitsOk = true;
				}

				if(serialBaudOk || serialBitsOk) // if serial settings were changed then restart program
				{
					my_ultoa(get_serial_baud_rate(userDBs.serialBaudIndex), szTemp, 10);
					_sprintf(str, "new_serial_settings:%s%c%d%d\r\n", szTemp, get_serial_parity(userDBs.serialParity),
																												 	 	 	 	 	  get_serial_data_bit(userDBs.serialData),
																																	  get_serial_stop_bit(userDBs.serialStop));
					CommsSendString(str,sci_get_current_port());
					while(true); // restart the program. nothing valid this point below(wait until watch dog reset the program)
					status = 22;		// magic 22 is responsible for write operation successful
				}
				status = 22;		// magic 22 is responsible for write operation successful
			}
			else
				status = 23;		// magic 23 is responsible for write operation fail

			break;
	}

	return status;
}

//char SerialInternalBaud(eRegMode mode, char *str)
//{
//	char status = 0;
//	char szTemp[25];
//	char *nonConvert;
//	char tempData[10];
//	uint8_t tempParitydIndex;
//	uint8_t tempDataBitIndex;
//	uint8_t tempStopBitIndex;
//	uint8_t tempBaudIndex;
//	uint32_t tempBaud;
//	bool serialBitsOk = false;
//	bool serialBaudOk = false;
//
//	switch (mode)
//	{
//		case REG_MODE_EXECUTE:
//			// nothing to execute
//			break;
//		case REG_MODE_READ:
//			my_ultoa(get_serial_baud_rate(userDBs.serialBaudIndex), szTemp, 10); // convert baud rate in to string
//			_sprintf(str, "%s%c%d%d\r\n", szTemp, get_serial_parity(userDBs.serialParity),
//																						get_serial_data_bit(userDBs.serialData),
//																						get_serial_stop_bit(userDBs.serialStop)); // combine baud + serial bits
//			status = 1;
//			break;
//		case REG_MODE_WRITE:
//			tempBaud = (uint32_t)strtoul(GetRegDataPointer(),&nonConvert,10);
//			strcpy(tempData,nonConvert);
//			serialBitsOk = (isSerialParityAvailable(tempData[0], &tempParitydIndex) &&
//											isSerialDataBitAvailable(tempData[1], &tempDataBitIndex) &&
//											isSerialStopBitAvailable(tempData[2], &tempStopBitIndex))? true : false; // check serial bits available
//			serialBaudOk = (isSerialBaudAvailable(tempBaud, &tempBaudIndex))? true : false; // check serial baud available
//			if(serialBitsOk && serialBaudOk) // if both bits and baud are real
//			{
//				serialBitsOk = false; // reset for another purpose
//				serialBaudOk = false; // reset for another purpose
//
//				if(userDBs.serialBaudIndex != tempBaudIndex) // write if the baud is change only else just "Ok"
//				{
//					userDBs.serialBaudIndex = tempBaudIndex;
//					WriteUserDB();
//					ReadUserDB();
//					serialBaudOk = true;
//				}
//
//				if(userDBs.serialParity != tempParitydIndex ||	userDBs.serialData != tempDataBitIndex || userDBs.serialStop != tempStopBitIndex)  // write if the data bits are change only else just "Ok"
//				{
//					userDBs.serialParity = tempParitydIndex;
//					userDBs.serialData = tempDataBitIndex;
//					userDBs.serialStop = tempStopBitIndex;
//					WriteUserDB();
//					ReadUserDB();
//					serialBitsOk = true;
//				}
//
//				if(serialBaudOk || serialBitsOk) // if serial settings were changed then restart program
//				{
//					my_ultoa(get_serial_baud_rate(userDBs.serialBaudIndex), szTemp, 10);
//					_sprintf(str, "new_serial_settings:%s%c%d%d\r\n", szTemp, get_serial_parity(userDBs.serialParity),
//																												 	 	 	 	 	  get_serial_data_bit(userDBs.serialData),
//																																	  get_serial_stop_bit(userDBs.serialStop));
//					CommsSendString(str,sci_get_current_port());
//					while(true); // restart the program. nothing valid this point below(wait until watch dog reset the program)
//					status = 22;		// magic 22 is responsible for write operation successful
//				}
//				status = 22;		// magic 22 is responsible for write operation successful
//			}
//			else
//				status = 23;		// magic 23 is responsible for write operation fail
//
//			break;
//	}
//
//	return status;
//}

char IODirection(eRegMode mode, char *str)
{
	char status = 0;
	char *nonConvert;
	uint8_t ioDir;

	switch (mode)
	{
	case REG_MODE_EXECUTE:
		// nothing to execute
		break;
	case REG_MODE_READ:
		_sprintf(str,"%d\r\n",(userDBs.ioDirection | 0x0F)); // 0x0F is due to for unconfigurable outputs
		SetConfigMode(false);		 	// turn off configuration mode irrespective of mode
		status = 1;
		break;
	case REG_MODE_WRITE:
		ioDir = (uint8_t)strtol(GetRegDataPointer(),&nonConvert,10);
		if((ioDir >= 0) && (ioDir <= 0XFF))
		{
			if(userDBs.ioDirection != ioDir)
			{
				userDBs.ioDirection = ioDir;
				WriteUserDB();
				ReadUserDB();
			}
			SetConfigMode(false);		 	// turn off configuration mode irrespective of mode
			status = 22; 							// write operation success
		}
		else
			status = 23;		// magic 23 is responsible for write operation fail
		break;
	}

	return status;
}
