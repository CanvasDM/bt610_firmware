//=================================================================================================
//!
//! @file AttributeValidator.h
//!
//! @brief The validators are common to the Transmitter and Info Board.
//!
//! @note These are called from the attribute task (mutex protected) and operate on the attribute 
//! table.
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
//=================================================================================================

#ifndef ATTRIBUTE_VALIDATOR_H
#define ATTRIBUTE_VALIDATOR_H

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

//=================================================================================================
// Global Data Definitions
//=================================================================================================


//=================================================================================================
// Global Function Prototypes
//=================================================================================================

int8_t AttributeValidator_Bypass(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_SpecialString(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_GenericString(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_TrimString(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_uint32_t(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_uint16_t(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_uint8_t(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_int32_t(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_int16_t(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_int8_t(uint32_t Index, void * pValue, size_t Length, bool DoWrite);
int8_t AttributeValidator_float(uint32_t Index, void * pValue, size_t Length, bool DoWrite);

#ifdef __cplusplus
}
#endif

#endif

// end
