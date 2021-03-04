/*
 * system.c
 *
 *  Created on: March 04, 2021
 *      Author: Janaka Madhusankha
 */
#include <stdbool.h>
#include "systems.h"
#include "clock.h"
#include "sci.h"

void idle_f(void)
{
	// idle_f loop
}

void bg_tick(void)
{

}

void bg_10(void)
{

}

void bg_100(void)
{

}

void bg_1000(void)
{

}

void idle(void)
{
  static bool running = false;
  static byte last_tenths, last_ticks, last_hundreds = 0;

  if (!running)
  {
    running = true;
    idle_f();

    if (get_time_out_status())
    {
      last_ticks++;
      set_time_out(false);
      bg_tick();
    }
    if (last_ticks >= 10)
    {
    	last_ticks = 0;
      last_tenths ++;
      bg_10();
    }
    if (last_tenths >= 10)
    {
    	last_tenths = 0;
      last_hundreds++;
      bg_100();
    }
    if (last_hundreds >= 10)
    {
    	last_hundreds = 0;
      bg_1000();
    }
    running = false;
  }
}
