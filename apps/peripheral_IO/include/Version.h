//=================================================================================================
//! @file Version.h
//!
//! @brief Version number of software in the format of  Major.Minor.Build.
//! 
//! Copyright (c) 2020 Laird Connectivity
//=================================================================================================

#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

//=================================================================================================
// Includes
//=================================================================================================

//=================================================================================================
// Global Constant, Macro, and Type Definitions
//=================================================================================================
#define mkstr(s)        #s 
#define DEVICE_NAME     CONFIG_BT_DEVICE_NAME
#define VERSION_MAJOR   1
#define VERSION_MINOR   0
#define VERSION_BUILD   1
#define VERSION_STRING  "1.0.1"

#define VERSION_BUILD_TIME   __DATE__", "__TIME__

#define TIMESTAMPED_VERSION  ""VERSION_STRING" "__DATE__" "__TIME__""

#ifdef DEBUG_BUILD
  #ifdef USE_PERCEPIO_TRACEALYZER
    #define BUILD_TYPE              "DEBUG w/TRACE"
  #else
    #define BUILD_TYPE              "DEBUG"
  #endif
#else
  #ifdef USE_PERCEPIO_TRACEALYZER
    #define BUILD_TYPE              "RELEASE w/TRACE"
  #else
    #define BUILD_TYPE              "RELEASE"
  #endif
#endif

#define VERSION_STRING_MAX_LENGTH (32)

//=================================================================================================

extern const char TIMESTAMPED_VERSION_STRING[VERSION_STRING_MAX_LENGTH];


#ifdef __cplusplus
}
#endif

#endif

// end
