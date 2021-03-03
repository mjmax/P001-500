#include <avr/io.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "eeprom.h"


// Write data to the eeprom address
void eepromWrite(uint16_t address, uint16_t length, const uint8_t *data)
{
  uint16_t i;
  uint8_t ch = 0;

  // for each byte, check if it needs changing before writing
  for (i = 0; i < length; i++)
  {
    eepromRead((uint16_t)(address + i), 1, &ch);
    if (ch != data[i])
    {
    	eeprom_write_byte_single((uint16_t)(address + i), data[i]);
    }
  }
}

// Read data from the eeprom address
void eepromRead(uint16_t address, uint16_t length, uint8_t *data)
{
  uint16_t i = 0;

  if(length == 1)
  	data[i] = (uint8_t)eeprom_read_byte_single(address);
  else
  {
  	// Read each byte
  	for (i = 0; i < length-1; i++)
  	{
  		data[i] = eeprom_read_byte_single(address + i);
  	}
  }
}


void eeprom_write_byte_single(uint16_t address, uint8_t value)
{
	/*  Flush buffer to make sure no unintetional data is written and load
	 *  the "Page Load" command into the command register.
	 */
	eeprom_flushbuffer();
	NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;

	/* Set address to write to. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0xFF;
	NVM.ADDR2 = 0x00;

	/* Load data to write, which triggers the loading of EEPROM page buffer. */
	NVM.DATA0 = value;

	/*  Issue EEPROM Atomic Write (Erase&Write) command. Load command, write
	 *  the protection signature and execute command.
	 */
	NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
	nvm_execute();
}


uint8_t eeprom_read_byte_single(uint16_t address)
{
	uint8_t value;

	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Set address to read from. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0xFF;
	NVM.ADDR2 = 0x00;

	/* Issue EEPROM Read command. */
	NVM.CMD = NVM_CMD_READ_EEPROM_gc;
	nvm_execute();

	value = NVM.DATA0;

	return value;
}


/*! \brief Write one byte to EEPROM using IO mapping.
 *
 *  This function writes one byte to EEPROM using IO-mapped access.
 *  If memory mapped EEPROM is enabled, this function will not work.
 *  This functiom will cancel all ongoing EEPROM page buffer loading
 *  operations, if any.
 *
 *  \param  pageAddr  EEPROM Page address, between 0 and EEPROM_SIZE/EEPROM_PAGESIZE
 *  \param  byteAddr  EEPROM Byte address, between 0 and EEPROM_PAGESIZE.
 *  \param  value     Byte value to write to EEPROM.
 */
void eeprom_writebyte( uint8_t pageAddr, uint8_t byteAddr, uint8_t value )
{
	/*  Flush buffer to make sure no unintetional data is written and load
	 *  the "Page Load" command into the command register.
	 */
	eeprom_flushbuffer();
	NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;

	/* Calculate address */
	uint16_t address = (uint16_t)(pageAddr*EEPROM_PAGESIZE)
	                            |(byteAddr & (EEPROM_PAGESIZE-1));

	/* Set address to write to. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0x1F;
	NVM.ADDR2 = 0x00;

	/* Load data to write, which triggers the loading of EEPROM page buffer. */
	NVM.DATA0 = value;

	/*  Issue EEPROM Atomic Write (Erase&Write) command. Load command, write
	 *  the protection signature and execute command.
	 */
	NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
	nvm_execute();
}


/*! \brief Read one byte from EEPROM using IO mapping.
 *
 *  This function reads one byte from EEPROM using IO-mapped access.
 *  If memory mapped EEPROM is enabled, this function will not work.
 *
 *  \param  pageAddr  EEPROM Page address, between 0 and EEPROM_SIZE/EEPROM_PAGESIZE
 *  \param  byteAddr  EEPROM Byte address, between 0 and EEPROM_PAGESIZE.
 *
 *  \return  Byte value read from EEPROM.
 */
uint8_t eeprom_readbyte( uint8_t pageAddr, uint8_t byteAddr )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Calculate address */
	uint16_t address = (uint16_t)(pageAddr*EEPROM_PAGESIZE)
	                            |(byteAddr & (EEPROM_PAGESIZE-1));

	/* Set address to read from. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0x1F;
	NVM.ADDR2 = 0x00;

	/* Issue EEPROM Read command. */
	NVM.CMD = NVM_CMD_READ_EEPROM_gc;
	nvm_execute();

	return NVM.DATA0;
}


/*! \brief Wait for any NVM access to finish, including EEPROM.
 *
 *  This function is blcoking and waits for any NVM access to finish,
 *  including EEPROM. Use this function before any EEPROM accesses,
 *  if you are not certain that any previous operations are finished yet,
 *  like an EEPROM write.
 */
void eeprom_waitfornvm( void )
{
	do {
		/* Block execution while waiting for the NVM to be ready. */
	} while ((NVM.STATUS & NVM_NVMBUSY_bm) == NVM_NVMBUSY_bm);
}


/*! \brief Flush temporary EEPROM page buffer.
 *
 *  This function flushes the EEPROM page buffers. This function will cancel
 *  any ongoing EEPROM page buffer loading operations, if any.
 *  This function also works for memory mapped EEPROM access.
 *
 *  \note The EEPROM write operations will automatically flush the buffer for you.
 */
void eeprom_flushbuffer( void )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Flush EEPROM page buffer if necessary. */
	if ((NVM.STATUS & NVM_EELOAD_bm) != 0) {
		NVM.CMD = NVM_CMD_ERASE_EEPROM_BUFFER_gc;
		nvm_execute();
	}
}


/*! \brief Load single byte into temporary page buffer.
 *
 *  This function loads one byte into the temporary EEPROM page buffers.
 *  If memory mapped EEPROM is enabled, this function will not work.
 *  Make sure that the buffer is flushed before starting to load bytes.
 *  Also, if multiple bytes are loaded into the same location, they will
 *  be ANDed together, thus 0x55 and 0xAA will result in 0x00 in the buffer.
 *
 *  \note Only one page buffer exist, thus only one page can be loaded with
 *        data and programmed into one page. If data needs to be written to
 *        different pages, the loading and writing needs to be repeated.
 *
 *  \param  byteAddr  EEPROM Byte address, between 0 and EEPROM_PAGESIZE.
 *  \param  value     Byte value to write to buffer.
 */
void eeprom_loadbyte( uint8_t byteAddr, uint8_t value )
{
	/* Wait until NVM is not busy and prepare NVM command.*/
	eeprom_waitfornvm();
	NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;

	/* Set address. */
	NVM.ADDR0 = byteAddr & 0xFF;
	NVM.ADDR1 = 0x00;
	NVM.ADDR2 = 0x00;

	/* Set data, which triggers loading of EEPROM page buffer. */
	NVM.DATA0 = value;
}


/*! \brief Load entire page into temporary EEPROM page buffer.
 *
 *  This function loads an entire EEPROM page from an SRAM buffer to
 *  the EEPROM page buffers. If memory mapped EEPROM is enabled, this
 *  function will not work. Make sure that the buffer is flushed before
 *  starting to load bytes.
 *
 *  \note Only the lower part of the address is used to address the buffer.
 *        Therefore, no address parameter is needed. In the end, the data
 *        is written to the EEPROM page given by the address parameter to the
 *        EEPROM write page operation.
 *
 *  \param  values   Pointer to SRAM buffer containing an entire page.
 */
void eeprom_loadpage( const uint8_t * values )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();
	NVM.CMD = NVM_CMD_LOAD_EEPROM_BUFFER_gc;

	/*  Set address to zero, as only the lower bits matters. ADDR0 is
	 *  maintained inside the loop below.
	 */
	NVM.ADDR1 = 0x00;
	NVM.ADDR2 = 0x00;

	/* Load multible bytes into page buffer. */
	for (uint8_t i = 0; i < EEPROM_PAGESIZE; ++i) {
		NVM.ADDR0 = i;
		NVM.DATA0 = *values;
		++values;
	}
}

/*! \brief Write already loaded page into EEPROM.
 *
 *  This function writes the contents of an already loaded EEPROM page
 *  buffer into EEPROM memory.
 *
 *  As this is an atomic write, the page in EEPROM will be erased
 *  automatically before writing. Note that only the page buffer locations
 *  that have been loaded will be used when writing to EEPROM. Page buffer
 *  locations that have not been loaded will be left untouched in EEPROM.
 *
 *  \param  pageAddr  EEPROM Page address, between 0 and EEPROM_SIZE/EEPROM_PAGESIZE
 */
void eeprom_automaticwritepage( uint8_t pageAddr )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Calculate page address */
	uint16_t address = (uint16_t)(pageAddr*EEPROM_PAGESIZE);

	/* Set address. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0x1F;
	NVM.ADDR2 = 0x00;

	/* Issue EEPROM Atomic Write (Erase&Write) command. */
	NVM.CMD = NVM_CMD_ERASE_WRITE_EEPROM_PAGE_gc;
	nvm_execute();
}


/*! \brief Erase EEPROM page.
 *
 *  This function erases one EEPROM page, so that every location reads 0xFF.
 *
 *  \param  pageAddr  EEPROM Page address, between 0 and EEPROM_SIZE/EEPROM_PAGESIZE
 */
void eeprom_erasepage( uint8_t pageAddr )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Calculate page address */
	uint16_t address = (uint16_t)(pageAddr*EEPROM_PAGESIZE);

	/* Set address. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0x1F;
	NVM.ADDR2 = 0x00;

	/* Issue EEPROM Erase command. */
	NVM.CMD = NVM_CMD_ERASE_EEPROM_PAGE_gc;
	nvm_execute();
}


/*! \brief Write (without erasing) EEPROM page.
 *
 *  This function writes the contents of an already loaded EEPROM page
 *  buffer into EEPROM memory.
 *
 *  As this is a split write, the page in EEPROM will _not_ be erased
 *  before writing.
 *
 *  \param  pageAddr  EEPROM Page address, between 0 and EEPROM_SIZE/EEPROM_PAGESIZE
 */
void eeprom_splitwritepage( uint8_t pageAddr )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Calculate page address */
	uint16_t address = (uint16_t)(pageAddr*EEPROM_PAGESIZE);

	/* Set address. */
	NVM.ADDR0 = address & 0xFF;
	NVM.ADDR1 = (address >> 8) & 0x1F;
	NVM.ADDR2 = 0x00;

	/* Issue EEPROM Split Write command. */
	NVM.CMD = NVM_CMD_WRITE_EEPROM_PAGE_gc;
	nvm_execute();
}

/*! \brief Erase entire EEPROM memory.
 *
 *  This function erases the entire EEPROM memory block to 0xFF.
 */
void eeprom_eraseall( void )
{
	/* Wait until NVM is not busy. */
	eeprom_waitfornvm();

	/* Issue EEPROM Erase All command. */
	NVM.CMD = NVM_CMD_ERASE_EEPROM_gc;
	nvm_execute();
}
