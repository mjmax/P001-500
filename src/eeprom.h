#ifndef EEPROM_H
#define EEPROM_H


#define EEPROM_PAGESIZE 32
#define EEPROM(_pageAddr, _byteAddr) \
	((uint8_t *) MAPPED_EEPROM_START)[_pageAddr*EEPROM_PAGESIZE + _byteAddr]



/* Definitions of macros. */

/*! \brief Enable EEPROM block sleep-when-not-used mode.
 *
 *  This macro enables power reduction mode for EEPROM.
 *  It means that the EEPROM block is disabled when not used.
 *  Note that there will be a penalty of 6 CPU cycles if EEPROM
 *  is accessed.
 */
#define eeprom_EnablePowerReduction() ( NVM.CTRLB |= NVM_EPRM_bm )

/*! \brief Disable EEPROM block sleep-when-not-used mode.
 *
 *  This macro disables power reduction mode for EEPROM.
 */
#define eeprom_DisablePowerReduction() ( NVM.CTRLB &= ~NVM_EPRM_bm )

/*! \brief Enable EEPROM mapping into data space.
 *
 *  This macro enables mapping of EEPROM into data space.
 *  EEPROM starts at EEPROM_START in data memory. Read access
 *  can be done similar to ordinary SRAM access.
 *
 *  \note This disables IO-mapped access to EEPROM, although page erase and
 *        write operations still needs to be done through IO register.
 */
#define eeprom_EnableMapping() ( NVM.CTRLB |= NVM_EEMAPEN_bm )

/*! \brief Disable EEPROM mapping into data space.
 *
 *  This macro disables mapping of EEPROM into data space.
 *  IO mapped access is now enabled.
 */
#define eeprom_DisableMapping() ( NVM.CTRLB &= ~NVM_EEMAPEN_bm )

/*! \brief Non-Volatile Memory Execute Command
 *
 *  This macro set the CCP register before setting the CMDEX bit in the
 *  NVM.CTRLA register.
 *
 *  \note The CMDEX bit must be set within 4 clock cycles after setting the
 *        protection byte in the CCP register.
 */
#define nvm_execute()	asm("push r30"      "\n\t"	\
			    "push r31"      "\n\t"	\
    			    "push r16"      "\n\t"	\
    			    "push r18"      "\n\t"	\
			    "ldi r30, 0xCB" "\n\t"	\
			    "ldi r31, 0x01" "\n\t"	\
			    "ldi r16, 0xD8" "\n\t"	\
			    "ldi r18, 0x01" "\n\t"	\
			    "out 0x34, r16" "\n\t"	\
			    "st Z, r18"	    "\n\t"	\
    			    "pop r18"       "\n\t"	\
			    "pop r16"       "\n\t"	\
			    "pop r31"       "\n\t"	\
			    "pop r30"       "\n\t"	\
			    )

/* Prototyping of functions. */

/*without EEPROM page access*/

void eepromWrite(uint16_t address, uint16_t length, const uint8_t *data);
void eepromRead(uint16_t address, uint16_t length, uint8_t *data);
void eeprom_write_byte_single(uint16_t address, uint8_t value);
uint8_t eeprom_read_byte_single(uint16_t address);

/*with EEPROM page access*/

void eeprom_writebyte( uint8_t pageAddr, uint8_t byteAddr, uint8_t value );
uint8_t eeprom_readbyte( uint8_t pageAddr, uint8_t byteAddr );
void eeprom_waitfornvm( void );
void eeprom_flushbuffer( void );
void eeprom_loadbyte( uint8_t byteAddr, uint8_t value );
void eeprom_loadpage( const uint8_t * values );
void eeprom_automaticwritepage( uint8_t pageAddr );
void eeprom_erasepage( uint8_t pageAddress );
void eeprom_splitwritepage( uint8_t pageAddr );
void eeprom_eraseall( void );

#endif
