/*
 * vectatxmega128d3.h -
 *
 * Header file for the remapped interrupt vector table for the Atxmega128d3
 * Suitable for use with the X300-506 bootloader
 *
 * Ian Burns 21/8/2015
 *
 * Interrupt vectors live in a section called .vectors located at 0x0
 *
 * Your reset vector function should be defined like so:
 *
 * int main(uint_farptr_t user_flash_func_ptr)
 *
 * The uint_farptr_t is used because the bootloader is outside the 128KByte
 * easily addressable range.
 *
 * To start the bootloader, call:
 * start_flash_programmer(user_flash_func_ptr);
 *
 */

#ifndef VECTATXMEGA128D3_DOT_H
#define VECTATXMEGA128D3_DOT_H 1

#include <inttypes.h>


#if 0
/** Program flash using UARTC, pass to @ref tUserFlashFn */
#define USER_FLASH_UARTC 0


/** Program flash using UARTC, pass to @ref tUserFlashFn */
#define USER_FLASH_UARTD 1


/** Program flash using UARTE, pass to @ref tUserFlashFn */
#define USER_FLASH_UARTE 2
#endif


/** Function typedef for function to call to initiate user flash programming from the application */
void start_flash_programmer(uint_farptr_t user_flash_func_ptr);


#endif /* #ifndef VECTATXMEGA128D3_DOT_H */
