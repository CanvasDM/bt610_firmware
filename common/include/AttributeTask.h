//=================================================================================================
//! @file AttributeTask.h
//!
//! @brief During init, the system should send MSG_CODE_INITIALIZE_ATTRIBUTES and then 
//!        wait for MSG_CODE_ATTRIBUTES_READY.
//!
//! @note Writing attributes to NV requires TBD ms.  During this time all writes/reads will 
//!       be blocked (by mutex).
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
//=================================================================================================

#ifndef ATTRIBUTE_TASK_H
#define ATTRIBUTE_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

//=================================================================================================
// Includes
//=================================================================================================
#include <stdint.h>
#include <stdbool.h>

//#include "FrameworkMsgConfiguration.h"

//#include "Attribute.h"

//=================================================================================================
// Global Constants, Macros and Type Definitions
//=================================================================================================

//typedef struct
//{
//  FrameworkMsgHeader_t header;
//  size_t count;
//  uint8_t list[ATTRIBUTE_TOTAL_KEYWORDS];  // not uint32_t to keep size down
  
//} AttributeChangedMsg_t;

// The task IDs are used to determine if an attribute is writable or not.
// Both the ackConfig and configuration file come from the network task.
#define NETWORK_FILE_SOURCE_ID 0xFE

// This is how long after a set occurs before the write and subsequent
// broadcast message will be sent.
#ifndef ATTRIBUTE_UPDATE_TICKS
  #define ATTRIBUTE_UPDATE_TICKS  MS_TO_TICKS(1000)
#endif

/* 
 min output range is -21474836.40 to 21474836.40 (12), but add decimal places for set function
 Type Size    Range (+/-)              Decimals Exponent Mantissa Alignment
 float 32 bits 1.18E-38 to 3.40E+38    7       8 bits   23 bits     4
*/
#define ATTRIBUTE_MAX_FLOAT_DIGITS  (20)
#define FLOAT_OUTPUT_SPECIFIER "%.2f"
#define FLOAT_INPUT_SPECIFIER  "%.7f"
/*
  These include space for the NUL character
*/
#define ATTRIBUTE_NAME_MAX_LENGTH   (40)
#define MAX_METHOD_LENGTH           (20+1)
#define MAX_VERSION_LENGTH          (11+1)    // major.minor.build (0-999)
#define MAX_HW_VERSION_LENGTH       (1+1)     // "0" - "9"
#define MAX_SENSOR_NAME_LENGTH      (23+1)
#define MAX_SENSOR_ID_LENGTH        (32+1)
#define PASSKEY_LENGTH              (6+1)
#define MAX_RESET_REASON_LENGTH     (9+1)
#define BLUETOOTH_ADDRESS_LENGTH    (12+1)

#define ATTRIBUTE_STRING_MAX_LENGTH (64+1) 
//=================================================================================================
// Global Data Definitions
//=================================================================================================


//=================================================================================================
// Global Function Prototypes
//=================================================================================================

//-----------------------------------------------
//! @brief
//!
void AttributeTask_Initialize(void);

//-----------------------------------------------
//! @brief  Used to get the value of a variable as a string.
//!
//! @param  pIndex - If retval is true, then this will be set to an index in the table.
//! @param  pName - The name (string) of a variable to find in the attribute table.  
//! @param  NameLength - The length of the name (strlen)
//!
//! @note   The name doesn't NOT need to be NUL terminated.
//!
//! @retval true if the name exists in the attribute table.
//!
bool AttributeTask_Match(uint32_t *pIndex, char const * pName, size_t NameLength);

//-----------------------------------------------
//! @brief  Used to set the value of a variable as a string.
//!
//! @param  Index - A valid index into attribute table.
//! @param  pValue - string representation of variable
//! @param ValueLength - The length (without null char) of the string being passed in.
//!
//! @retval true if the value could be set with the value provided
//!
bool AttributeTask_SetWithString(uint32_t Index, char const * pValue, size_t ValueLength);

//-----------------------------------------------
//! @brief  Used to restore an attribute to its default value
//!
//! @param  Index - A valid index into attribute table.
//!
//! @retval true if the value is writable from the SourceId interface
//!
bool AttributeTask_RestoreDefaultValue(uint32_t Index, uint32_t SourceId);

//-----------------------------------------------
//! @brief  Used to set the value of an integer attribute.
//!
//! @param  Index - A valid index into attribute table.
//! @param  Value - The value to set.
//!
//! @retval true if the value could be set with the value provided, 
//!         false if it couldn't or the value isn't an integer.
//!
bool AttributeTask_SetUint32(uint32_t Index, uint32_t Value);
bool AttributeTask_SetSigned32(uint32_t Index, int32_t Value);

//-----------------------------------------------
//! @brief  Used to set the value of a floating point attribute
//!
//! @param  Index - A valid index into attribute table.
//! @param  Value - The value to set.
//!
//! @retval true if the value could be set with the value provided, 
//!         false if it couldn't or the value isn't an float
//!
bool AttributeTask_SetFloat(uint32_t Index, float Value);


//-----------------------------------------------
//! @brief  Used to set the version number.
//!
//! @param  Index - A valid index into attribute table.
//! @note   Minor and Build are ignored for hardware versions.
//!
//! @retval true if the value could be set, 
//!         false if the index isn't for a version
//!
bool AttributeTask_SetVersion(uint32_t Index, uint32_t Major, uint32_t Minor, uint32_t Build);

//-----------------------------------------------
//! @brief  Used to get the value of a variable as a string.
//!
//! @param  pValue - String representation of variable.
//!                  Argument must be at least MaxStringLength long.
//!
//! @param  pIsNotString - true if the value is an integer or float, 
//!                        false if it is a string
//! @param  Index - A valid index into attribute table
//! @param  MaxStringLength - The maximum length of the string provided to 
//!         this function (including NUL).
//!
//! @retval The size of the returned string (strlen, NUL not included).  
//!         If the string isn't long enough, then nothing will be copied.
//!
size_t AttributeTask_GetValueAsString(char * pValue, 
                                      bool * pIsNotString, 
                                      uint32_t Index, 
                                      size_t MaxStringLength);


//-----------------------------------------------
//! @brief  Used to get the value of a variable as a string without IsNotString parameter.
//!
//! @ref AttributeTask_GetValueAsString
//!
size_t AttributeTask_GetString(char * pValue, uint32_t Index, size_t MaxStringLength);

//-----------------------------------------------
//! @brief  Used to get the value of a variable as an unsigned/signed integer.
//!
//! @param  pValue, 0 if attribute is a string.
//! @param  Index - A valid index into attribute table
//!
//! @retval false if the value wasn't found or isn't the correct type, true otherwise.
//!
bool AttributeTask_GetUint32(uint32_t * pValue, uint32_t Index);
bool AttributeTask_GetSigned32(int32_t * pValue, uint32_t Index);


//-----------------------------------------------
//! @brief  Used to get the value of a variable as float.
//!
//! @param  pValue, 0 if attribute is a string.
//! @param  Index - A valid index into attribute table
//!
//! @retval false if the value wasn't found or isn't a float, true otherwise.
//!
bool AttributeTask_GetFloat(float * pValue, uint32_t Index);

//-----------------------------------------------
//! @param  pName - NUL terminated string/name of variable associated with index.
//!                 This must be a string of at least ATTRIBUTE_NAME_MAX_LENGTH.
//!
//! @param  Index - a valid index into table
//!
//! @note Used to build JSON BLE response.
//! 
void AttributeTask_GetName(char * pName, uint32_t Index);

//-----------------------------------------------
//! @param  Index - A valid index into the attribute table   
//!
//! @retval true if the index is associated with a string, otherwise false
//!
bool AttributeTask_IsString(uint32_t Index);

//-----------------------------------------------
//! @param  Index - A valid index into the attribute table   
//!
//! @retval true if the index is associated with a float, otherwise false
//!
bool AttributeTask_IsFloat(uint32_t Index);

//-----------------------------------------------
//! @param  Index - A valid index into the attribute table   
//!
//! @retval true if the index is associated with a unsigned int, otherwise false
//!
bool AttributeTask_IsUnsigned(uint32_t Index);

//-----------------------------------------------
//! @param  Index - A valid index into the attribute table   
//!
//! @retval true if the index is associated with a signed int, otherwise false
//!
bool AttributeTask_IsSigned(uint32_t Index);


//-----------------------------------------------
//! @param  Index - A valid index into the attribute table
//! @param  SourceId - Task id of the request
//!
//! @retval true if the value is writable (including lock flag), false otherwise
//!
bool AttributeTask_IsWritable(uint32_t Index, uint32_t SourceId);

//-----------------------------------------------
//! @retval true if the value has a category of read write, false otherwise
//!
bool AttributeTask_IsReadWrite(uint32_t Index);

//-----------------------------------------------
//! @retval true if the value has a category of read only, false otherwise
//!
bool AttributeTask_IsReadOnly(uint32_t Index);

//-----------------------------------------------
//! @retval true if the value has a category of protocol, false otherwise
//!
bool AttributeTask_IsProtocol(uint32_t Index);


//-----------------------------------------------
//! @brief This is for estimation purposes only!
//!
size_t AttributeTask_GetMaxPairSize(uint32_t Index);


#ifdef __cplusplus
}
#endif

#endif

// end
