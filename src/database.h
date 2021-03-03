/*
 * database.h
 *
 *	base on the T700-512 database.c file and modify to suitable with AtXmega128D3 MC
 *  Created on: Mar 27, 2020
 *      Author: Janaka Madhusankha
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#include <string.h>
#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>
#include "hardware.h"
#include "command.h"
#include "outputs.h"

enum eDatabaseCopy {DatabaseCopy0 = 0, DatabaseCopy1, NumDatabaseCopies};

#define APP_DB_MODEL_STR_LEN 7
#define APP_DB_ID_STR_LEN 17

// EEPROM Locations for both copies of each database
#define FACTORY_DB_1_ADDRESS   0
#define FACTORY_DB_2_ADDRESS 100
#define USER_DB_1_ADDRESS    200
#define USER_DB_2_ADDRESS    600
// Next item should start at location 1000, we have only a total of 2k to use

#define NV_SCRATCHPAD_LEN 100

// NV RAM used by test code only
// Volitile to application code
// Working copies of the Oven and Functional test
// data are held here.  Must be written to flash to
// persist.
typedef struct
{
    uint8_t eutaddress;
    unsigned char nv_scratchpad[NV_SCRATCHPAD_LEN];
} DB;


/*
 * Factory database
 */
typedef struct {
  uint32_t serialNo;
  char modelStr[APP_DB_MODEL_STR_LEN];
  bool tradeEn;

  uint32_t chksum;                // checksum will be checked before database use
} FactoryDB;



/*
 * Application user database
 */
typedef struct {
  // Passcodes
  uint32_t safePasscode;
  uint32_t fullPasscode;
  // Serial settings
  uint8_t serialAddress;
  uint8_t serialBaudIndex;
  uint8_t serialParity;
  uint8_t serialData;
  uint8_t serialStop;

  uint8_t writeReplyAck;
  uint8_t ioDirection;
//  uint8_t cofSetting;
//  uint8_t cofMode;  // TODO is this one needed?
  // ID string
  char id[APP_DB_ID_STR_LEN];

  uint32_t chksum;                // checksum will be checked before database use
} UserDB;

extern FactoryDB factoryDBs;

extern UserDB userDBs;

extern DB dbs;

/** Factory database version - increase this whenever the factory database (@ref FactoryDB) is changed */
#define FACTORY_DB_VER             0x01

/** Application user database version - increase this whenever the application user database (@ref UserDB) is changed */
#define APP_USER_DB_VER            0x01

/** Checksum seed for for the factory application database */
#define FACTORY_DB_CHECKSUM_SEED ( sizeof( FactoryDB ) + FACTORY_DB_VER )

/** Checksum seed for the application database */
#define APP_USER_DB_CHECKSUM_SEED ( sizeof( UserDB ) + APP_USER_DB_VER )


#define APP_DB_U_SAFEPASS_MIN 0
#define APP_DB_U_SAFEPASS_MAX (DISPLAY_INT_MAX)
#define APP_DB_U_SAFEPASS_DEFAULT 0

#define APP_DB_U_FULLPASS_MIN 0
#define APP_DB_U_FULLPASS_MAX (DISPLAY_INT_MAX)
#define APP_DB_U_FULLPASS_DEFAULT 0

#define APP_DB_U_SERADDRESS_MIN 0
#define APP_DB_U_SERADDRESS_MAX 31
#define APP_DB_U_SERADDRESS_DEFAULT 31

#define APP_DB_U_SERBAUD_MIN 0
#define APP_DB_U_SERBAUD_MAX (NUM_DB_BAUD_SETTINGS - 1)
#define APP_DB_U_SERBAUD_DEFAULT DB_SERIAL_BAUDIDX_9600

#define APP_DB_U_SERPARITY_MIN 0
#define APP_DB_U_SERPARITY_MAX (NUM_DB_PARITY_SETTINGS - 1)
#define APP_DB_U_SERPARITY_DEFAULT DB_SERIAL_PARITYIDX_NONE

#define APP_DB_U_SERDATA_MIN 0
#define APP_DB_U_SERDATA_MAX (NUM_DB_DATA_SETTINGS - 1)
#define APP_DB_U_SERDATA_DEFAULT DB_SERIAL_DATAIDX_8

#define APP_DB_U_SERSTOP_MIN 0
#define APP_DB_U_SERSTOP_MAX (NUM_DB_STOP_SETTINGS - 1)
#define APP_DB_U_SERSTOP_DEFAULT DB_SERIAL_STOPIDX_1

#define APP_DB_U_WRITEREPLYACK_MIN 0
#define APP_DB_U_WRITEREPLYACK_MAX (MAX_WRITE_ACK - 1)
#define APP_DB_U_WRITEREPLYACK_DEFAULT WRITE_ACK_ENABLE

#define APP_DB_U_IODIR_MIN 0
#define APP_DB_U_IODIR_MAX OUTPUT_DIR
#define APP_DB_U_IODIR_DEFAULT OUTPUT_DIR

//#define APP_DB_U_SERCOF_MIN 0
//#define APP_DB_U_SERCOF_MAX (COF_SETTING_NUM - 1)
//#define APP_DB_U_SERCOF_DEFAULT COF_SETTING3_ASCII_WEIGHT
//
//#define APP_DB_U_SERCOF_MODE_MIN 0
//#define APP_DB_U_SERCOF_MODE_MAX (NUM_COF_MODES - 1)
//#define APP_DB_U_SERCOF_MODE_DEFAULT COF_MODE_OFF

#define APP_DB_U_SERID_MAXLEN APP_DB_ID_STR_LEN
#define APP_DB_U_SERID_DEFAULT "PW6 MCR5 10kg"

#define APP_DB_U_CHKSUM_MINIMUM (0)
#define APP_DB_U_CHKSUM_MAXIMUM (UINT16_MAX)
#define APP_DB_U_CHKSUM_DEFAULT 0


void InitUserDBs(void);
void WriteUserDB(void);
bool ReadUserDB(void);
void DefaultUserDB(void);

void InitFactoryDBs(void);
void WriteFactoryDB(void);
bool ReadFactoryDB(void);
void DefaultFactoryDB(void);

void InitAppDBs(void);

void InitTestDBs(void);
void DefaultTestDB(void);

////....................Support functions to calculate checksum..............////////

/**
 * Calculate a 16bit CCITT CRC checksum for a block of data
 *
 * @param byt is an array of bytes to calculate the checksum for
 * @param size is the number of bytes to calculate the checksum over
 * @param crc is the checksum seed
 * @return the checksum of the given data
 */
uint16_t AdcCalcCRC16(const uint8_t *byt, size_t size, uint16_t crc);

/** Define to calculate a database checksum. Relies on db containing a member "chksum" */
#define DB_CHECKSUM(dbtype, db, seed) \
  AdcCalcCRC16((const uint8_t *) &(db), offsetof(dbtype, chksum), seed)

/** Define to calculate and set the checksum within a database. Relies on db containing a member "chksum" */
#define DB_SET_CHECKSUM(dbtype, db, seed) \
  (db).chksum = DB_CHECKSUM(dbtype, db, seed)

/** Define to return true if the checksum on a database is good. Relies on db containing a member "chksum" */
#define DB_CHECKSUM_GOOD(dbtype, db, seed) \
  (DB_CHECKSUM(dbtype, db, seed) == (db).chksum)

#endif /* DATABASE_H_ */
