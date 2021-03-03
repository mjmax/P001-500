/*
 * outputs.h
 *
 *  Created on: 12/10/2015
 *      Author: IanB
 */

#ifndef OUTPUTS_H_
#define OUTPUTS_H_

#include <stdbool.h>
#include <stdint.h>
#include "inputs.h"

#define OUTPUT_DIR	0x3F			// default io direction
#define FIXED_OUTPUT_MASK	0x0F	// first 4 output are fixed

#define COMMS_DRIVEN_OUTPUT			4
//#define DIRECT_DRIVEN_OUTPUT		NUMBER_OF_OUTPUT - COMMS_DRIVEN_OUTPUT

#define OUT1 0
#define OUT2 1
#define OUT3 2
#define OUT4 3

#define OUT1LED 4
#define OUT2LED 5
#define OUT3LED 6

typedef enum {LED_OUT_OFF = 0, LED_OUT_OVER, LED_OUT_UNDER, LED_OUT_PASS, MAX_LED_STATUS}ledOutput_t;


void outputs_init(void);


void outputs_set(uint8_t output, bool level);


#endif /* OUTPUTS_H_ */
