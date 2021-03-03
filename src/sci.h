/*
 * sci.h
 *
 *  Created on: 05/03/2019
 *      Author: Janaka Madhusankha
 */

#ifndef SCI_H_
#define SCI_H_

#include <stdbool.h>
#include <stdio.h>

#define BUFFER_SIZE 254
#define SPECILA_CHAR_BUFFER_SIZE 20

#define arlen(array) (sizeof((array))/sizeof((array)[0]))

//sci port definition
typedef enum {SCI_PORT_BT = 0, SCI_PORT_RS232 , SCI_PORT_INTERNAL, MAX_SCI_PORT} eSciPort;

typedef enum {CMD_PORT = 0, INTERNAL_PORT, EXTERNAL_BT_PORT, EXTERNAL_RS232_PORT, MAX_CMD_PORT}cmdPort;

typedef enum {CHAR_ENCODED_IO = 1, CHAR_ENCODED_DAC}char_encoded_msg;

typedef enum {DAC_TYPE_VOLTAGE = 0, DAC_TYPE_CURRENT, DAC_TYPE_MAX}dac_type;

typedef struct {
	uint8_t SCBuffer[SPECILA_CHAR_BUFFER_SIZE]; // special character store
	uint8_t pointer;		// pointed character up to now
	uint8_t free; // free buffer space
}sc_buffer; // this buffer is to store special characters (use it as FIFO buffer)

typedef unsigned char   byte;

extern FILE sci_strm[MAX_SCI_PORT];

extern char message[BUFFER_SIZE];

extern sc_buffer specialCH;

#define PORT_CMD 0x30
#define PORT_BT 0x31
#define PORT_EXT 0x32
#define PORT_R320 0x33
#define SCI_DEFAULT_PORT SCI_PORT_RS232
#define SPECIAL_CHAR_MASK	0x80

#define SPECIAL_CHAR_DAC_TYPE_MASK			0x03
#define SPECIAL_CHAR_DAC_TYPE_VOLTAGE		0x01
#define SPECIAL_CHAR_DAC_TYPE_CURRENT		0x02

#define SPECIAL_CHAR_TYPE_IO		0x80   //(10xxxxxx uses for send output status)
#define SPECIAL_CHAR_TYPE_DAC		0xC0	 //(11xxxxxx uses for send output status (get 3 characters to form 16 bit number))
#define SPECIAL_CHAR_TYPE_MASK	0xC0


void init_sc_buffers(void);
void init_sc_buffer(sc_buffer *buffer);
bool put_char_sc_buffer(sc_buffer *buff, uint8_t ch);
uint8_t get_last_char_sc_buffer(sc_buffer *buff);
uint8_t get_any_char_sc_buffer(sc_buffer *buff, uint8_t pointer);
uint8_t get_sc_buffer_level(sc_buffer *buff);
void checkEncodedChar(void);
void DecodeSpecilaCharacters(char ch);
void extractOutputValue(char ch);
byte GetSpecialChatacterType(char ch);
bool checkForSpecialCharacters(char *ch);

void SendDac(uint16_t dacValue,byte type);

eSciPort sci_get_current_port(void);
void sci_set_current_port(eSciPort sci);
void RinNetMsgProcess(void);
void DefaultMsgProcess(void);
void CommsSendString(char *reply, eSciPort port);
void LoadMessage(eSciPort port);
bool sci_get_new_message(eSciPort sci);
void get_rx_buffer(eSciPort sci, char *msg);
void sci_rx_clear(eSciPort sci);
void MultiplexSciPort(eSciPort sci);
void PutCharactor(char ch, FILE *stream);
void init_sci_stream(void);
FILE *get_stream(eSciPort sci);
void init_sc_buffers(void);

#endif /* SCI_H_ */
