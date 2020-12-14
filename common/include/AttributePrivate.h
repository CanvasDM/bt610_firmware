//=================================================================================================
//!
//! @file AttributePrivate.h
//!
//! @brief This is "private" and should only be called by the Attribute implementation files.
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
//=================================================================================================

#ifndef ATTRIBUTE_PRIVATE_H
#define ATTRIBUTE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

//=================================================================================================
// Includes
//=================================================================================================
#include <stdint.h>
#include <stdbool.h>

//=================================================================================================
// Global Constants, Macros and Type Definitions
//=================================================================================================

typedef enum
{
  RESEVED_CATEGORY = 0,
  READ_WRITE = 1,
  READ_ONLY = 2,
  PROTOCOL = 3
    
} AttributeCategory_t;

typedef enum
{
  UNSIGNED_TYPE = 0,
  SIGNED_TYPE,
  FLOAT_TYPE,
  STRING_TYPE
    
} AttributeType_t;

//
// These are used to make the table smaller.
//
#define DRW DEFAULT_RW_ATTRIBUTE_VALUES
#define DRO DEFAULT_RO_ATTRIBUTE_VALUES

#define u UNSIGNED_TYPE  
#define i SIGNED_TYPE    
#define f FLOAT_TYPE     
#define s STRING_TYPE    

#define b true // backup
#define y true // broadcast-yes

typedef struct AttributeEntryTag
{
  const char * name;
  void * pData;
  void const * const pDefault;
  const size_t size;
  const AttributeCategory_t category;
  const AttributeType_t type;
  const bool backup;  // not factory resetable
  const bool lockable;
  const bool broadcast;
  int8_t (*pValidator)(uint32_t, void *, size_t, bool);
  const uint32_t min;
  const uint32_t max;
  bool modified;
  
} AttributeEntry_t;

//=================================================================================================
//

// Print all of the values that change 
#ifndef VERBOSE_WRITE_LIST
  #define VERBOSE_WRITE_LIST (true)
#endif

//=================================================================================================
// Global Data Definitions
//=================================================================================================

//=================================================================================================
// Global Function Prototypes
//=================================================================================================

void Attribute_Initialize(void);
void * Attribute_GetRwPointer(void);  // get pointer to read-write data that needs to be stored in NV.
size_t Attribute_GetRwSize(void);
size_t Attribute_GetSize(void);
uint32_t Attribute_GetMaxHashValue(void);
void Attribute_CopyDefaults(void * pDefaults, size_t Size);
void Attribute_SetNonBackupValuesToDefault(void);
bool Attribute_DeprecationHandler(void);
void Attribute_SecondaryInitialization(void);

uint32_t Attribute_Hash(const char *str, size_t len);

#ifdef __cplusplus
}
#endif

#endif

// end
