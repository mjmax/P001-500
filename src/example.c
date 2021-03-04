#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "vectatxmega128d3.h"
#include "usart_int.h"
#include "hardware.h"
#include "wand.h"
#include "r300.h"
#include "outputs.h"
#include "inputs.h"
#include "asciidef.h"
#include "command.h"
#include "registers.h"
#include "sci.h"
#include "_sprintf.h"
#include "eeprom.h"
#include "database.h"
#include "clock.h"
#include "systems.h"


static uint_farptr_t the_user_flash_func_ptr = NULL;


#if 0
#define MENU_BUFFER_LEN 20
static char menu_buffer[MENU_BUFFER_LEN];
static size_t posn = 0;

static void send_menu(FILE *stream)
{
  fprintf(stream, "Menu:\r\n"
                  "  print - Send a string\r\n"
                  "  flash - Start flash programming\r\n"
                  "  reboot - Reboot the unit\r\n"
                  "  r300 - Send \"20050003:;\" to the R300\r\n"
                  "  output - Drive outputs eg: output 1 1\r\n");
}


/** Run a simple menu */
static bool handle_menu_data(FILE *stream, uint8_t ch, FILE *r300stream)
{
  bool handled = false;
  unsigned int output_num;
  unsigned int output_lvl;

  if (posn >= MENU_BUFFER_LEN - 1)
  {
    posn = 0;
    fprintf(stream, "Command too long\r\n");
    return false;
  }

  menu_buffer[posn++] = ch;
  menu_buffer[posn] = '\0';

  if (ch == '\r' || ch == '\n')
  {
    /* Dispatch */
    menu_buffer[posn - 1] = '\0';
    handled = true;

    if (0 == strcmp(menu_buffer, "print"))
    {
      fprintf(stream, "Printing a test string\r\n");
    }
    else if (0 == strcmp(menu_buffer, "flash"))
    {
      start_flash_programmer(the_user_flash_func_ptr);
    }
    else if (0 == strcmp(menu_buffer, "reboot"))
    {
      /* DMT will reset us */
      while (1)
        ;
    }
    else if (0 == strcmp(menu_buffer, "r300"))
    {
      fprintf(stream, "Sending command to R300\r\n");
      fprintf(r300stream, "20050003:;");
    }
    else if (2 == sscanf(menu_buffer, "output %u %u", &output_num, &output_lvl))
    {
      fprintf(stream, "Setting output %u to %u\r\n", output_num, output_lvl);
      outputs_set(output_num, 0 != output_lvl);
    }
    else
    {
      handled = false;
    }

    posn = 0;
    send_menu(stream);
  }
  return handled;
}
#endif


static void HandleData(void)
{
	byte count = 0;

	//checkEncodedChar(); // check if there is any special characters in internal stream (characters above 0x7F)
	for(count = 0; (count < MAX_SCI_PORT) ;count++) // process all messages in one loop priority to internal
	{
		if(sci_get_new_message(count))
		{
			MultiplexSciPort(count);
			sci_rx_clear(count);
		}
	}
}

int main(uint_farptr_t user_flash_func_ptr)
{
  sei();
  InitAppDBs();
  init_hardware();
  IntTimers();
  SetConfigMode(false);
  the_user_flash_func_ptr = user_flash_func_ptr;
  //send_menu(&usart2_stream);
  while (true)
  {
  	idle();
  	if(!IsConfigMode())
  		AutomaticSyncData();

    HandleData();
    wdt_reset();
  }

  return 0;
}
