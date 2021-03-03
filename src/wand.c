/*
 * wand.c
 *
 *  Created on: 24/09/2015
 *      Author: IanB
 */
#include <stdio.h>

#include "wand.h"


void handle_wand_data(FILE *stream, uint8_t ch)
{
  fputc(ch, stream);
}
