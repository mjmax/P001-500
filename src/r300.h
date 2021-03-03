/*
 * r300.h
 *
 *  Created on: 12/10/2015
 *      Author: IanB
 */

#ifndef R300_H_
#define R300_H_

#include <stdio.h>
#include <stdint.h>


void handle_r300_data(FILE *stream, uint8_t ch);

// this function sync data with R320 when special flag pop up
void AutomaticSyncData(void);

#endif /* R300_H_ */
