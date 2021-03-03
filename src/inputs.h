/*
 * inputs.h
 *
 *  Created on: 12/10/2015
 *      Author: IanB
 */

#ifndef INPUTS_H_
#define INPUTS_H_

#include <stdbool.h>

#define MAX_IO						8		// do not change

// this is for configurable IO (IO 5 (PA0), 6(PA1) can be configure either input or output change the mask then it will automatically configure)
// Configured as 2 input 6 output mode
//#define NUMBER_OF_INPUT		2			// (max: 4)
#define POART_A_MAX_INP		0x0F  // max input mask
//#define NUMBER_OF_OUTPUT	MAX_IO - NUMBER_OF_INPUT	// (max: 6)

typedef struct{
		uint8_t internal;
		uint8_t external;
}input_t;

void inputs_init(void);
void init_read_port_A(void);
input_t get_inputs(void);
void SetInputInterruptFlag(bool flag);
bool GetInputInterruptFlag(void);


#endif /* INPUTS_H_ */
