/*
 * command.h
 *
 *  Created on: 27/04/2018
 *      Author: Janaka Madhusankha
 */

#ifndef REGISTERS_H_
#define REGISTERS_H_

#include <string.h>
#include <stdbool.h>
#include <string.h>
#include "Command.h"
#include "strings.h"
#include "asciidef.h"
#include "inputs.h"
#include "sci.h"

#define MODEL_REG	 						0x00
#define STATUS_REG						0x01
#define CONFIG_TYPE_REG	 			0x02
#define CONFIG_DATA_REG  			0x03
#define SOFTWARE_VERSION_REG	0x04
#define SERIAL_NO_REG  				0x05

//#define USART_EXTERNAL_REG		0x07
#define BUZZER_STATUS_REG			0x08
#define OUTPUT_LED_STATUS_REG 0x09
#define INPUT_STATUS_REG  		0x0A
#define OUTPUT_STATUS_REG  		0x0B
#define ANALOGE_OUTPUT_REG		0x0C
#define END_REG								0xFF		// do not use this register for any application


//.............CONFIG_REGISTERS...............//
#define CANCEL_CONFIG_MODE								0x00

#define CONFIG_WRITE_REPLY_ACK						0x01
#define CONFIG_USART_EXTERNAL							0x02
//#define CONFIG_USART_INTERNAL							0x03
#define CONFIG_IO_DIRECTIONS							0x04

#define END_CONFIG												0xFF		// do not use this register for any application


#define arrayLength(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))
#define lowerCaseToUpperCase(charactor) ((char)(((int)(charactor)) - 20))
#define upperCaseToLowerCase(charactor) ((char)(((int)(charactor)) + 20))

typedef enum {REG_MODE_EXECUTE = 0, REG_MODE_READ, REG_MODE_WRITE, REG_MODE_MAX}regMode;

typedef unsigned short eRegId;

typedef byte eRegMode;

typedef char (*t_testfn_ptr)(eRegMode mode, char *str);

struct rinReg_t
{
    eRegId reg;
    eRegMode mode;
    t_testfn_ptr testfn;
};

#define REG_READABLE		(byte)REG_ACCESSIBILITY_R_STRING
#define REG_WRITABLE		(byte)REG_ACCESSIBILITY_W_STRING

///////.............combined register mode...................//////
/* combined register modes created by adding individual register modes
 * ex: if 0x02 reg is capable of read and wire the combined reg mode value (RRW) = (REG_READABLE + REG_WRITABLE)
 *
 * (RXX) is the short form of reg combined mode and XX denote Read capability and write capability
 * "_" denote null capability
 */
#define RRW	(REG_READABLE + REG_WRITABLE)
#define RR_	(REG_READABLE)
#define R_W	(REG_WRITABLE)


bool IsConfigMode(void);
void SetConfigMode(bool staus);
void GetRegValue(byte addres ,char *data);

//...........................REGISTER_FUNCTIONS.......................//
char SerialNumber(eRegMode mode, char *str);
char CurrentStatus(eRegMode mode, char *str);
char ConfigType(eRegMode mode, char *str);
char ConfigData(eRegMode mode, char *str);
char SoftwareVersion(eRegMode mode, char *str);
char CopyrightString(eRegMode mode, char *str);
char ModelNumber(eRegMode mode, char *str);
char OutputLedStatus(eRegMode mode, char *str);
char InputStatus(eRegMode mode, char *str);
char OutputStatus(eRegMode mode, char *str);
char AnalogeOutput(eRegMode mode, char *str);
char SerialBaudRate(eRegMode mode, char *str);
char BuzzerStatus(eRegMode mode, char *str);
char NullTest(eRegMode mode, char *str);

//...........................CONFIG_FUNCTIONS.......................//
char CancelConfigMode(eRegMode mode, char *str);
char WriteReplyAck(eRegMode mode, char *str);
char SerialExternalBaud(eRegMode mode, char *str);
//char SerialInternalBaud(eRegMode mode, char *str);
char IODirection(eRegMode mode, char *str);

#endif /* REGISTERS_H_ */
