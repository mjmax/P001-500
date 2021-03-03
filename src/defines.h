#ifndef DEFINES_DOT_H
#define DEFINES_DOT_H 1

/*
 * Define one ONLY processor type giving the source code a conditional to work with
 *
 * This is done in the makefile now
 *
 */


/*
 * It would be unwise to set debug to 1, since the extra code this compiles
 * in would push the bootloader over the 2k mark, and into user flash space.
 */
#define DEBUG 0


/*
 * The entire set of defines are replicated for each processor to save having
 * to wade through code to find relevant stuff.
 *
 * I've done away with this. The stuff below is relevant to the processor
 *
 */


#ifdef __AVR_ATxmega128D3__

/*
 * Unit Specific Constants
 */
#define USER_BASE 0x00000000UL
#define USER_LENGTH 0x00020000UL
#define USER_BLOCKSIZE 0x00001000UL
#define USER_CHKSUM 0x8000

#define MODULE_VERSION "P001-500-0.01  " /* Software Version - Don't change this comment, used by Makefile to extract version */


/*
 * Size of flash programming block (how much the processor can program in 1 hit)
 */
#define FLASH_LINE_SIZE SPM_PAGESIZE

#endif /* #ifdef AVR_ATxmega128D3 */


/*
 * The stuff below here is common to all bootloaders
 *
 * DO NOT CHANGE
 */


/*
 * Size of each block transferred to the bootloader to be programmed
 */
#define SERIAL_BLOCK_SIZE 0x0080


/*
 * Write/Verify Flash Responses (sent by bootloader to PC):
 */
#define WRITE_REQUEST_MORE_DATA 0x00
#define WRITE_FLASH_VERIFY_OK 0x22
#define WRITE_FLASH_FAILURE 0x23
#define ERASE_COMPLETE 0x24
#define ERASE_FAILURE 0x25


/*
 * Write/Verify Flash Commands (sent by PC to bootloader):
 */
#define WRITE_CANCEL 0xFF

/*
 * Encryption Parameters
 */
#define IV_PRESET_LO 0x5E89CFB5
#define IV_PRESET_HI 0x9A31AE87

#define KEY_LO 0x5F713C19
#define KEY_MID_LO 0xEEBA917B
#define KEY_MID_HI 0x64B16401
#define KEY_HI 0x8BA93F71

/*
 * Commands
 */
#define CMD_WRITE 0xB7D6F53C
#define CMD_VERIFY 0x53E1A8D7
#define CMD_ERASE 0xD768FEFA
#define CMD_IDENTIFY 0x63DE3D96
#define CMD_RESET 0xC33723C6


#endif /* #ifndef DEFINES_DOT_H */
