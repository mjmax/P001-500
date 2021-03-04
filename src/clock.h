/*
 * clock.h
 *
 *  Created on: 16/09/2015
 *      Author: IanB
 */

#ifndef CLOCK_H_
#define CLOCK_H_


#define CLOCK_FREQ 18432000UL


void initMicroSecTimer(void);

void IntTimers(void);
uint16_t clockMicroSec(void);


#endif /* CLOCK_H_ */
