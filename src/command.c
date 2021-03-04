/*
 * command.c
 *
 *  Created on: 19/04/2018
 *      Author: Janaka Madhusankha
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <avr/wdt.h>
#include "_sprintf.h"
#include "command.h"
#include "asciidef.h"
#include "hardware.h"
#include "registers.h"
#include "strings.h"
#include "database.h"

TempValueDB appVariables;		// this is only for X350 special variable store
char regDataArray[COMMAND_BUFFER_LENGTH-MIN_CMD_LENGTH];
static commandDetails zeroStreamCommand;

byte char_to_byte(char ch)
{
	const char elementArr[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	bool itemFound = false;
	byte index = 0;
	byte finalValue = 0;

	for(index = 0; index < arlen(elementArr); index++)
	{
		if(((byte)elementArr[index] - (byte)ch) == 0)
		{
			itemFound = true;
			finalValue = index;
			break;
		}
	}

	if(!itemFound)
		finalValue = 0xFF; // set to maximum

	return finalValue;
}

void SetConfigAddress(byte add)
{
	appVariables.configueAddress = add;
}

byte GetConfigAddress(void)
{
	return appVariables.configueAddress;
}

bool IsMessageValid(char *command)
{
  bool validity = false;
  char commandMessage[COMMAND_BUFFER_LENGTH];
  char *checkA;
  char *checkB;
  int checkC;
  char *checkD;

  strncpy(commandMessage, command, arlen(commandMessage));
  checkA = strchr(commandMessage,PROTOCOL_SEPERATOR);
  checkB = strchr(commandMessage,END_OF_LINE);
  checkC = strlen(commandMessage);
  checkD = strchr(commandMessage,LINE_SEPERATOR);
  if((checkA != NULL) && (checkB != NULL) && (checkC >= MIN_CMD_LENGTH) && (checkD != NULL))
    validity = true;

  return validity;
}

void SeperateCommand(commandDetails *pHeader, char command[])
{
	pHeader->add = command[0];
	pHeader->cmd = command[1];
	pHeader->reg = command[2];
	if((command[4] == LINE_SEPERATOR) && (command[5] == END_OF_LINE))
	{
		pHeader->dataAvailable = false;
	}
	else
	{
		for (int count = 0; count<(COMMAND_BUFFER_LENGTH-MIN_CMD_LENGTH);count++)
		{
			if (command[4+count] == LINE_SEPERATOR)
			{
				pHeader->hexData[count] = '\0';
				break;
			}
			pHeader->hexData[count] = command[4+count];
		}
		pHeader->dataAvailable = true;
	}
}

char *GetRegDataPointer(void)
{
	return regDataArray;
}

void UpdateWriteDataArray(void)
{
//	byte strLen;
	memset(regDataArray, '\0', arrayLength(regDataArray));
	strcpy(regDataArray,zeroStreamCommand.hexData);

//	strLen = strlen(regDataArray) - 1;

	if(regDataArray[0] == '\0' || regDataArray[0] == '\r' || regDataArray[0] == '\n' || regDataArray[0] == ';') // no data to update
		regDataArray[0] = 0; // set data as zero to avoid corruption
//	else if(regDataArray[strLen] == ';') // if line end with comma
//		regDataArray[strLen] = '\0'; // remove the comma and add a end character
//	else
//	{
//		if(regDataArray[strLen] == '\n')// if line end with LF character
//		{
//			regDataArray[strLen] = '\0';	// remove the LF character and add end character
//			if(regDataArray[strLen - 1] == '\r')// if line end with CRLF character
//				regDataArray[strLen - 1] = '\0';	// remove the CR character and add end character
//		}
//	}
}

void MakeHeader(commandDetails *pHeader , char *message)
{
	char add = pHeader->add;

	if(pHeader->add != PROTOCOL_BROADCAST_MSG)
		add = (char)((byte)add + MAX_MODULE_COUNT);

	_sprintf(message, "%c%c%c%c%c:", H_DLE , PORT_CMD, add, pHeader->cmd, pHeader->reg); // prototype header (always reply address is 9) "<DLE>09XX:"
}

void ReadReg(byte regId)
{
	char szTemp[10];
	char szReply[40];
	char reply[arlen(szReply) + arlen(szTemp)];

	MakeHeader(&zeroStreamCommand ,szTemp);
	GetRegValue(regId ,szReply);
	memset(reply, '\0', arlen(reply));
	_sprintf(reply,"%s%s",szTemp,szReply);
	CommsSendString(reply,sci_get_current_port());
}

void WriteReg(byte regId)
{
	char szTemp[10];
	char szReply[40];
	char reply[arlen(szReply) + arlen(szTemp)];

	UpdateWriteDataArray();
	GetRegValue(regId ,szReply);
	if(userDBs.writeReplyAck == WRITE_ACK_ENABLE)
	{
		MakeHeader(&zeroStreamCommand ,szTemp);
		memset(reply, '\0', arlen(reply));
		_sprintf(reply,"%s%s",szTemp,szReply);
		CommsSendString(reply,sci_get_current_port());
	}
}

void SetIncomingRegMode(byte regMode)
{
	appVariables.incomingRegMode = regMode;
}

byte GetIncomingRegMode(void)
{
	return appVariables.incomingRegMode;
}

bool IsMessageForMe(void)
{
	bool status = false;

	switch(zeroStreamCommand.add)
	{
		case PROTOCOL_BROADCAST_MSG:
		case PROTOCOL_X350_REAR_MSG:
			status = true;
			break;
	}
	return status;
}

// main command handling function
void CommandHandle(char command[])
{
  bool ok, sendOutMessage = false,  allMessageProcessed = false, firstTime = true;
  char oneCommand[EX_IO_MODULE_MAX][20], test[20], outMessage[MAX_MODULE_COMM_BUFFER];
  byte count, strLen;
  eSciPort currentSciPort;
  int begine = 0;
	int msgCount = 0;
	int poniCount = 0;

  memset(oneCommand,'\0',sizeof(oneCommand));
	for(count = 0; count <= strlen(command); count++) // search for DLEs
	{
		oneCommand[msgCount][poniCount++] = command[count];
		if(command[count] == END_OF_LINE)
			break;
		if(command[count] == MESSAGE_SEPERATOR)
		{
			oneCommand[msgCount][poniCount - 1] = LINE_SEPERATOR;
			oneCommand[msgCount][poniCount] = END_OF_LINE;
			msgCount += 1; 	// more than one message
			poniCount = 0;
		}
	}
	begine = msgCount;

	while (!allMessageProcessed)
	{
		memset(test,'\0',arrayLength(test));
		strcpy(test, &oneCommand[(begine - msgCount)][0]);

    ok = IsMessageValid(test);
    if (ok)
    {
    	wdt_reset();
    	SeperateCommand(&zeroStreamCommand, test);
    	if(IsMessageForMe())
    	{
    		SetIncomingRegMode((byte)zeroStreamCommand.cmd);
    		switch((byte)zeroStreamCommand.cmd)
    		{
    		case REG_READABLE:
    			ReadReg(char_to_byte(zeroStreamCommand.reg));
    			break;
    		case REG_WRITABLE:
    			WriteReg(char_to_byte(zeroStreamCommand.reg));
    			break;
    		}
    	}
    	else
    	{
    		sendOutMessage = true;

    		if(firstTime)
    		{
    			memset(outMessage, '\0', arrayLength(outMessage));
    			outMessage[0] = (char)H_DLE;
    			outMessage[1] = (char)PORT_CMD;
    			firstTime = false;
    		}

    		strLen = strlen(test);
    		test[0] = (char)((byte)test[0] - 1);
    		test[strLen - 1] = '\0';
    		test[strLen - 2] = MESSAGE_SEPERATOR;
    		strcat(outMessage, test);
    	}
    }

		if(msgCount <= 0)
			allMessageProcessed = true;

		msgCount -= 1;
	}

  if(sendOutMessage)
  {
  	strLen = strlen(outMessage);
  	outMessage[strLen - 1] = LINE_SEPERATOR;
  	outMessage[strLen] = END_OF_LINE;

  	currentSciPort = sci_get_current_port();
  	sci_set_current_port(SCI_PORT_RS232);
  	CommsSendString(outMessage,sci_get_current_port());
  	sci_set_current_port(currentSciPort);
  }

}
