/*
 * database.c
 *
 *	base on the T700-512 database.c file and modify to suitable with AtXmega128D3 MC
 *  Created on: Mar 27, 2020
 *      Author: Janaka Madhusankha
 *
 */

#include <avr/io.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "database.h"
#include "eeprom.h"



FactoryDB factoryDBs;

UserDB userDBs;

DB dbs;

const uint16_t factoryDatabaseAddress[NumDatabaseCopies] = {
    FACTORY_DB_1_ADDRESS,FACTORY_DB_2_ADDRESS
};

const uint16_t userDatabaseAddress[NumDatabaseCopies] = {
    USER_DB_1_ADDRESS, USER_DB_2_ADDRESS
};

static const UserDB AppDefaultUserDb =
{
  //.adcUserDB = AppDefaultAdcUserDB,

  .safePasscode = APP_DB_U_SAFEPASS_DEFAULT,
  .fullPasscode = APP_DB_U_FULLPASS_DEFAULT,

  .serialAddress = APP_DB_U_SERADDRESS_DEFAULT,
  .serialBaudIndex = APP_DB_U_SERBAUD_DEFAULT,
  .serialParity = APP_DB_U_SERPARITY_DEFAULT,
  .serialData = APP_DB_U_SERDATA_DEFAULT,
  .serialStop = APP_DB_U_SERSTOP_DEFAULT,
	.writeReplyAck = APP_DB_U_WRITEREPLYACK_DEFAULT,
	.ioDirection = APP_DB_U_IODIR_DEFAULT,
//  .cofSetting = APP_DB_U_SERCOF_DEFAULT,
//  .cofMode= APP_DB_U_SERCOF_MODE_DEFAULT,
  .id = APP_DB_U_SERID_DEFAULT,

  .chksum = APP_DB_U_CHKSUM_DEFAULT,
};


bool ReadFactoryDB(void)
{
  bool res = false;
  uint8_t i;

  for (i = 0; i < NumDatabaseCopies; i++ )
  {
    eepromRead(factoryDatabaseAddress[i], sizeof(FactoryDB), (uint8_t *)&factoryDBs);
    if (DB_CHECKSUM_GOOD( FactoryDB, factoryDBs, FACTORY_DB_CHECKSUM_SEED))
    {
      res = true;
      break;
    }
  }
  return res;
}


void WriteFactoryDB(void)
{
  uint8_t i;

  DB_SET_CHECKSUM( FactoryDB, factoryDBs, FACTORY_DB_CHECKSUM_SEED );

  for (i = 0; i < NumDatabaseCopies; i++ )
  {
    eepromWrite(factoryDatabaseAddress[i], sizeof(FactoryDB), (uint8_t *)&factoryDBs);
  }
}

void DefaultFactoryDB(void)
{
  memset( &factoryDBs, 0, sizeof( FactoryDB ) );
}

void InitFactoryDBs(void)
{

	if (!ReadFactoryDB())
  {
		DefaultFactoryDB();
  }
}

bool ReadUserDB(void)
{
  bool res = false;
  uint8_t i;

  for (i = 0; i < NumDatabaseCopies; i++ )
  {
    eepromRead(userDatabaseAddress[i], sizeof(UserDB), (uint8_t *)&userDBs);
    if (DB_CHECKSUM_GOOD( UserDB, userDBs, APP_USER_DB_CHECKSUM_SEED ))
    {
      res = true;
      break;
    }
  }
  return res;
}


void WriteUserDB(void)
{
  uint8_t i;

  DB_SET_CHECKSUM( UserDB, userDBs, APP_USER_DB_CHECKSUM_SEED );

  for (i = 0; i < NumDatabaseCopies; i++ )
  {
    eepromWrite(userDatabaseAddress[i], sizeof(UserDB), (uint8_t *)&userDBs);
  }
}

void DefaultUserDB(void)
{
  memset( &userDBs, 0, sizeof( UserDB ) );
  userDBs = AppDefaultUserDb;
}

void InitUserDBs(void)
{
	if (!ReadUserDB())
	{
		DefaultUserDB();
	}
}

void DefaultTestDB(void)
{
  memset( &dbs, 0, sizeof( DB ) );
}

void InitTestDBs(void)
{
	DefaultTestDB();
}

void InitAppDBs(void)
{
  InitFactoryDBs();
  InitUserDBs();
  InitTestDBs();
}

////....................Support functions to calculate checksum..............////////
/**
 * Calculate a 16bit CCITT CRC checksum for a block of data
 *
 * @param byt is an array of bytes to calculate the checksum for
 * @param size is the number of bytes to calculate the checksum over
 * @param crc is the checksum seed
 * @return the checksum of the given data
 */
uint16_t AdcCalcCRC16(const uint8_t *byt, size_t size, uint16_t crc)
{
  /*
   * Calculate CRC-16 value; uses The CCITT-16 Polynomial,
   * expressed as X^16 + X^12 + X^5 + 1
   */
  size_t index;
  uint8_t b;

  for (index = 0; index < size; ++index)
  {
    crc ^= (((uint16_t) byt[index]) << 8);
    for (b = 0; b < 8; ++b)
    {
      if (crc & 0x8000)
        crc = (crc << 1) ^ 0x1021;
      else
        crc = (crc << 1);
    }
  }
  return crc;
}
