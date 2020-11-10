//=================================================================================================
//!
#define THIS_FILE "AttributeFunctions.c"
//!
//!
//! @note psysart to pyend regions are generated by attributes.py.  
//!       The Python script requires an Excel spreadsheet of properties as an input and 
//!       gperf to be installed.
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
//=================================================================================================


//=================================================================================================
// Includes
//=================================================================================================
#include <zephyr.h>
#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "BspSupport.h"
#include "AttributeFunctions.h"

//=================================================================================================
// Local Constant, Macro and Type Definitions
//=================================================================================================

// pystart - hash info
// pyend

#ifndef ATTRIBUTE_TABLE_SIZE
  #define ATTRIBUTE_TABLE_SIZE (MAX_HASH_VALUE + 1)
#endif

#if ATTRIBUTE_TABLE_SIZE != (MAX_HASH_VALUE + 1)
  //#error "Invalid Attribute Table Size"
#endif

#if MAX_WORD_LENGTH >= ATTRIBUTE_NAME_MAX_LENGTH
  //#error "Attribute Name Too Small"
#endif

#if ATTRIBUTE_TOTAL_KEYWORDS != TOTAL_KEYWORDS
  //#error "Header doesn't match source"
#endif

#define BYPASS_STRING_LENGTH 2

//=================================================================================================
// Add things to the end!  Do not remove items.

typedef struct RwAttributesTag
{
    // pystart - rw attributes
  char sensorName[23+1];
  char sensorLocation[32+1];
  uint16_t advertisingInterval;
  uint16_t advertisingDuration;
  char passkey[6+1];
  uint8_t lock;
  uint16_t batterySenseInterval;
  uint16_t temperatureSenseInterval;
  uint8_t temperatureAggregationCount;
  uint16_t digitalOutput1Mv;
  uint16_t digitalOutput2Mv;
  uint8_t digitalInput1;
  uint8_t digitalInput2;
  uint8_t analogInput1Type;
  uint8_t analogInput2Type;
  uint8_t analogInput3Type;
  uint8_t analogInput4Type;
  int16_t highTemp1Thresh1;
  int16_t highTemp1Thresh2;
  int16_t lowTemp1Thresh1;
  int16_t lowTemp1Thresh2;
  uint16_t temp1DeltaThresh;
  int16_t highTemp2Thresh1;
  int16_t highTemp2Thresh2;
  int16_t lowTemp2Thresh1;
  int16_t lowTemp2Thresh2;
  uint16_t temp2DeltaThresh;
  int16_t highTemp3Thresh1;
  int16_t highTemp3Thresh2;
  int16_t lowTemp3Thresh1;
  int16_t lowTemp3Thresh2;
  uint16_t temp3DeltaThresh;
  int16_t highTemp4Thresh1;
  int16_t highTemp4Thresh2;
  int16_t lowTemp4Thresh1;
  int16_t lowTemp4Thresh2;
  uint16_t temp4DeltaThresh;
  uint16_t highAnalog1Thresh1;
  uint16_t highAnalog1Thresh2;
  uint16_t lowAnalog1Thresh1;
  uint16_t lowAnalog1Thresh2;
  uint16_t analog1DeltaThresh;
  uint16_t highAnalog2Thresh1;
  uint16_t highAnalog2Thresh2;
  uint16_t lowAnalog2Thresh1;
  uint16_t lowAnalog2Thresh2;
  uint16_t analog2DeltaThresh;
  uint16_t highAnalog3Thresh1;
  uint16_t highAnalog3Thresh2;
  uint16_t lowAnalog3Thresh1;
  uint16_t lowAnalog3Thresh2;
  uint16_t analog3DeltaThresh;
  uint16_t highAnalog4Thresh1;
  uint16_t highAnalog4Thresh2;
  uint16_t lowAnalog4Thresh1;
  uint16_t lowAnalog4Thresh2;
  uint16_t analog4DeltaThresh;
  uint8_t activeMode;
  uint8_t useCodedPhy;
  int8_t txPower;
  uint16_t networkId;
  uint8_t configVersion;
  uint8_t hardwareMinorVersion;
  uint8_t ledTestActive;
    // pyend
} RwAttribute_t;
//static_assert( ((sizeof(RwAttribute_t) % 4) == 0), "FDS requires 32-bit sized data");

static const RwAttribute_t DEFAULT_RW_ATTRIBUTE_VALUES =
{
  // pystart - rw defaults
  .sensorName = "BT610",
  .sensorLocation = "",
  .advertisingInterval = 1000,
  .advertisingDuration = 0,
  .passkey = "123456",
  .lock = 0,
  .batterySenseInterval = 0,
  .temperatureSenseInterval = 0,
  .temperatureAggregationCount = 1,
  .digitalOutput1Mv = 0,
  .digitalOutput2Mv = 0,
  .digitalInput1 = 0,
  .digitalInput2 = 0,
  .analogInput1Type = 0,
  .analogInput2Type = 0,
  .analogInput3Type = 0,
  .analogInput4Type = 0,
  .highTemp1Thresh1 = 127,
  .highTemp1Thresh2 = 127,
  .lowTemp1Thresh1 = -127,
  .lowTemp1Thresh2 = -127,
  .temp1DeltaThresh = 255,
  .highTemp2Thresh1 = 127,
  .highTemp2Thresh2 = 127,
  .lowTemp2Thresh1 = -127,
  .lowTemp2Thresh2 = -127,
  .temp2DeltaThresh = 255,
  .highTemp3Thresh1 = 127,
  .highTemp3Thresh2 = 127,
  .lowTemp3Thresh1 = -127,
  .lowTemp3Thresh2 = -127,
  .temp3DeltaThresh = 255,
  .highTemp4Thresh1 = 127,
  .highTemp4Thresh2 = 127,
  .lowTemp4Thresh1 = -127,
  .lowTemp4Thresh2 = -127,
  .temp4DeltaThresh = 255,
  .highAnalog1Thresh1 = 4096,
  .highAnalog1Thresh2 = 4096,
  .lowAnalog1Thresh1 = 0,
  .lowAnalog1Thresh2 = 0,
  .analog1DeltaThresh = 4096,
  .highAnalog2Thresh1 = 4096,
  .highAnalog2Thresh2 = 4096,
  .lowAnalog2Thresh1 = 0,
  .lowAnalog2Thresh2 = 0,
  .analog2DeltaThresh = 4096,
  .highAnalog3Thresh1 = 4096,
  .highAnalog3Thresh2 = 4096,
  .lowAnalog3Thresh1 = 0,
  .lowAnalog3Thresh2 = 0,
  .analog3DeltaThresh = 4096,
  .highAnalog4Thresh1 = 4096,
  .highAnalog4Thresh2 = 4096,
  .lowAnalog4Thresh1 = 0,
  .lowAnalog4Thresh2 = 0,
  .analog4DeltaThresh = 4096,
  .activeMode = 0,
  .useCodedPhy = 0,
  .txPower = 0,
  .networkId = 0,
  .configVersion = 0,
  .hardwareMinorVersion = 0,
  .ledTestActive = 0
  // pyend
};

typedef struct RoAttributesTag
{
  // pystart - ro attributes
  int32_t temperatureAll;
  int32_t temperatureResult1;
  int32_t temperatureResult2;
  int32_t temperatureResult3;
  int32_t temperatureResult4;
  uint8_t currentReadingMa;
  uint16_t batteryVoltageMv;
  uint8_t digitalInput1Alarm;
  uint8_t digitalInput2Alarm;
  uint8_t highTemperature1Alarm;
  uint8_t lowTemperature1Alarm;
  uint8_t deltaTemperature1Alarm;
  uint8_t highTemperature2Alarm;
  uint8_t lowTemperature2Alarm;
  uint8_t deltaTemperature2Alarm;
  uint8_t highTemperature3Alarm;
  uint8_t lowTemperature3Alarm;
  uint8_t deltaTemperature3Alarm;
  uint8_t highTemperature4Alarm;
  uint8_t lowTemperature4Alarm;
  uint8_t deltaTemperature4Alarm;
  uint8_t highAnalog1Alarm;
  uint8_t lowAnalog1Alarm;
  uint8_t deltaAnalog1Alarm;
  uint8_t highAnalog2Alarm;
  uint8_t lowAnalog2Alarm;
  uint8_t deltaAnalog2Alarm;
  uint8_t highAnalog3Alarm;
  uint8_t lowAnalog3Alarm;
  uint8_t deltaAnalog3Alarm;
  uint8_t highAnalog4Alarm;
  uint8_t lowAnalog4Alarm;
  uint8_t deltaAnalog4Alarm;
  char hwVersion[1+1];
  char firmwareVersion[11+1];
  char resetReason[8+1];
  char bluetoothAddress[12+1];
  uint8_t mtu;
  uint32_t flags;
  uint16_t resetCount;
  uint8_t digitalInput1Mv;
  uint8_t digitalInpu21Mv;
  int16_t lowTemp3Thresh2c;
  uint8_t magnetState;
  // pyend
} RoAttribute_t;

static const RoAttribute_t DEFAULT_RO_ATTRIBUTE_VALUES =
{
  // pystart - ro defaults
  .temperatureAll = 0,
  .temperatureResult1 = 0,
  .temperatureResult2 = 0,
  .temperatureResult3 = 0,
  .temperatureResult4 = 0,
  .currentReadingMa = 0,
  .batteryVoltageMv = 0,
  .digitalInput1Alarm = 0,
  .digitalInput2Alarm = 0,
  .highTemperature1Alarm = 0,
  .lowTemperature1Alarm = 0,
  .deltaTemperature1Alarm = 0,
  .highTemperature2Alarm = 0,
  .lowTemperature2Alarm = 0,
  .deltaTemperature2Alarm = 0,
  .highTemperature3Alarm = 0,
  .lowTemperature3Alarm = 0,
  .deltaTemperature3Alarm = 0,
  .highTemperature4Alarm = 0,
  .lowTemperature4Alarm = 0,
  .deltaTemperature4Alarm = 0,
  .highAnalog1Alarm = 0,
  .lowAnalog1Alarm = 0,
  .deltaAnalog1Alarm = 0,
  .highAnalog2Alarm = 0,
  .lowAnalog2Alarm = 0,
  .deltaAnalog2Alarm = 0,
  .highAnalog3Alarm = 0,
  .lowAnalog3Alarm = 0,
  .deltaAnalog3Alarm = 0,
  .highAnalog4Alarm = 0,
  .lowAnalog4Alarm = 0,
  .deltaAnalog4Alarm = 0,
  .hwVersion = "",
  .firmwareVersion = "",
  .resetReason = "",
  .bluetoothAddress = "",
  .mtu = 20,
  .flags = 0,
  .resetCount = 0,
  .digitalInput1Mv = 0,
  .digitalInpu21Mv = 0,
  .lowTemp3Thresh2c = -127,
  .magnetState = 0
  // pyend
};

//=================================================================================================
// Local Data Definitions
//=================================================================================================

static RwAttribute_t rw;
static RoAttribute_t ro;

//=================================================================================================
// Local Function Prototypes
//=================================================================================================
bool AttributeValidator_Passkey(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
bool AttributeValidator_TxPower(uint32_t Index, void * pValue, size_t Length, bool DoWrite);

//=================================================================================================
// Global Data Definitions
//=================================================================================================

// Expander for string and other values
//
//...............name...value...default....size..category
#define RW_ATTRS(n) STR(n), rw.n, DRW.n, sizeof(rw.n), READ_WRITE
#define RW_ATTRX(n) STR(n), &rw.n, &DRW.n, sizeof(rw.n), READ_WRITE
#define RO_ATTRS(n) STR(n), ro.n, DRO.n, sizeof(ro.n), READ_ONLY
#define RO_ATTRX(n) STR(n), &ro.n, &DRO.n, sizeof(ro.n), READ_ONLY
#define RP_ATTRS(n) STR(n), ro.n, DRO.n, sizeof(ro.n), PROTOCOL
#define RP_ATTRX(n) STR(n), &ro.n, &DRO.n, sizeof(ro.n), PROTOCOL

#define ATTR_UNUSED "\0" , NULL , NULL , 0, RESEVED_CATEGORY, u , 0, 0, 0, NULL, 0, 0 

// If min == max then range isn't checked.
#ifdef STILL_WORKING_ON
//index.....name.......................................type.backup.lockable.broadcast.validator.................min...max.
EXTERNED AttributeEntry_t attrTable[ATTRIBUTE_TABLE_SIZE] =
{
  // pystart - attribute table
  // pyend
};

//=================================================================================================
// Global Function Definitions
//=================================================================================================

void Attribute_Initialize(void)
{
  memcpy(&rw, &DRW, sizeof(RwAttribute_t));
  memcpy(&ro, &DRO, sizeof(RoAttribute_t));
}

void * Attribute_GetRwPointer(void)
{
  return &rw;
}

size_t Attribute_GetRwSize(void)
{
  return sizeof(RwAttribute_t);
}

size_t Attribute_GetSize(void)
{
  return (sizeof(RwAttribute_t) + sizeof(RoAttribute_t));
}

void Attribute_SecondaryInitialization(void)
{
}

void Attribute_PopulateScratchpads(void)
{
}

bool Attribute_DeprecationHandler(void)
{   
  return false;  
}

void Attribute_SetNonBackupValuesToDefault(void)
{
  size_t i = 0;
  for( i = 0; i < ATTRIBUTE_TABLE_SIZE; i++ )
  {
    if( !attrTable[i].backup )
    {  
      memcpy(attrTable[i].pData, attrTable[i].pDefault, attrTable[i].size);
    }
  }
}

//=================================================================================================
// Attribute Hash Table (allows near constant time lookup of names)
//=================================================================================================

uint32_t Attribute_GetMaxHashValue(void)
{
  //return MAX_HASH_VALUE;
}

uint32_t Attribute_Hash(const char *str, size_t len)
{
  static uint8_t asso_values[] =
  {
    // pystart - asso values
    // pyend
  };
  
  // pystart - hash function
  // pyend
  else
  {
    //return (MAX_HASH_VALUE + 1);
  }
}

//=================================================================================================
// Local Function Definitions
//=================================================================================================

bool AttributeValidator_Passkey(uint32_t Index, void * pValue, size_t Length, bool DoWrite)
{
  AttributeEntry_t *pEntry = &attrTable[Index];
  char * p = (char *)pValue;
  
  if( Length == (PASSKEY_LENGTH-1) ) // -1 to account for NUL
  {
    size_t i;
    for( i = 0; i < Length; i++ )
    {
      if( (p[i] < '0') || (p[i] > '9') )
      {
        return false;
      }
    }
    
    // Don't use strncmp because pValue isn't NUL terminated when coming from JSON
    if( DoWrite 
       && ( (memcmp(pEntry->pData, pValue, Length) != 0) ) )
    {
      pEntry->modified = true;
      memset(pEntry->pData, 0, pEntry->size);
      strncpy(pEntry->pData, pValue, Length);
    }
    return true;
  }
  return false;
}

bool AttributeValidator_TxPower(uint32_t Index, void * pValue, size_t Length, bool DoWrite)
{
  UNUSED_PARAMETER(Length);
  AttributeEntry_t *pEntry = &attrTable[Index];
  int32_t value = *((int32_t*)pValue);

  // Values supported by nRF52840
  bool valid = false;
  switch( value )
  {
  case -40: 
  case -20: 
  case -16: 
  case -12: 
  case  -8: 
  case  -4: 
  case   0: 
  case   2: 
  case   3: 
  case   4: 
  case   5: 
  case   6: 
  case   7: 
  case   8:
    valid = true;
    break;
    
  default:
    valid = false;
    break;
  }
  
  if( valid )
  {
    if( DoWrite
       && value != *((int32_t*)pEntry->pData) )
    {
      pEntry->modified = true;
      *((int32_t*)pEntry->pData) = value;
    }
    return true;
  }
  return false; 
}
#endif
// end
