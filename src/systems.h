/*
 * system.h
 *
 *  Created on: Feb 11, 2020
 *      Author: rinstrum
 */

#ifndef SYSTEMS_H_
#define SYSTEMS_H_

typedef unsigned short word;

void idle(void);

void idle_f(void);
void bg_tick(void);
void bg_10(void);
void bg_100(void);
void bg_1000(void);


#endif /* SYSTEMS_H_ */
