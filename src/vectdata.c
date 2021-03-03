/*
 * vectdata.c
 *
 *  Created on: 17/09/2015
 *      Author: IanB
 */
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "vectdata.h"


void *get_data_for_vector(int vectnum)
{
  if (vectnum >= NUM_VECTORS)
    return NULL;
  return pgm_read_ptr(&vector_data[vectnum]);
}
