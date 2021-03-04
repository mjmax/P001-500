/*
 * clock.h
 *
 *  Created on: 16/09/2015
 *      Author: IanB
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <stdbool.h>
#include <stdio.h>

#define CLOCK_FREQ 18432000UL

#define MILISECONS	1
#define CENTISECOND	10
#define DECISECOND	100
#define SECOND			1000

void IntTimers(void);
void IntSystemTimer(void);
void initMicroSecTimer(void);

void reset_timer(void);
bool get_time_out_status(void);
void set_time_out(bool status);
uint16_t clockMicroSec(void);


#endif /* CLOCK_H_ */
