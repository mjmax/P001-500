/*
 * vectdata.h
 *
 *  Created on: 17/09/2015
 *      Author: IanB
 */

#ifndef VECTDATA_H_
#define VECTDATA_H_

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define NUM_VECTORS (_VECTORS_SIZE / _VECTOR_SIZE)


/** Declare this table in your application */
extern void *const vector_data[NUM_VECTORS] PROGMEM;


void *get_data_for_vector(int vectnum);


#endif /* VECTDATA_H_ */
