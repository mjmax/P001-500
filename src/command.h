/*
 * command.h
 *
 *  Created on: 19/04/2018
 *      Author: Janaka Madhusankha
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <stdio.h>
#include "registers.h"
#include "sci.h"

// number of modules can be connected series
#define MAX_MODULE_COUNT 8

#define PROTOCOL_SEPERATOR (':')
#define MESSAGE_SEPERATOR  (',')
#define END_OF_LINE ('\n')
#define LINE_SEPERATOR ('\r')
#define PROTOCOL_X350_REAR_MSG  ('1')
#define PROTOCOL_BROADCAST_MSG ('0')

#define MAX_CMD_LENGTH 8
#define MIN_CMD_LENGTH 6
#define MAX_HEADER_LENGTH 3
#define COMMAND_BUFFER_LENGTH 35
#define REPLY_BUFFER_LENGTH 254
#define MAX_MODULE_COMM_BUFFER	65

typedef enum {WRITE_ACK_DISABLE = 0, WRITE_ACK_ENABLE, MAX_WRITE_ACK}writeReplyAck_t;

typedef struct{
 bool dataAvailable;
 char add;
 char cmd;
 char reg;
 char hexData[COMMAND_BUFFER_LENGTH-MIN_CMD_LENGTH];
}commandDetails;

typedef struct{
	uint8_t inputStatus;
	uint8_t outputStatus;
	uint8_t buzzerStatus;
	uint16_t analogOutStatus;
	uint8_t ledOutStatus;
}TempStatus_t;

typedef struct {
	TempStatus_t status;
	byte incomingRegMode;
	eSciPort currentSciPort;
	byte configueAddress;
	uint32_t chksum;                // checksum will be checked before database use
}TempValueDB;

typedef enum {
	EX_IO_MODULE_1 = 0,		// X300-007 board is first module and its fixed
	EX_IO_MODULE_2,
	EX_IO_MODULE_3,
	EX_IO_MODULE_4,
	EX_IO_MODULE_5,
	EX_IO_MODULE_6,
	EX_IO_MODULE_7,
	EX_IO_MODULE_MAX
}moduleCount_t; // note that X350 rear board is considered as the first IO module and so on


extern TempValueDB appVariables;		// this is only for X350 special variable store

void SetConfigAddress(byte add);
byte GetConfigAddress(void);

byte GetIncomingRegMode(void);
void SetIncomingRegMode(byte regMode);
char *GetRegDataPointer(void);

// main command handling function
void CommandHandle(char command[]);

#endif /* COMMAND_H_ */
