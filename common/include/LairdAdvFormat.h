/******************************************************************************/
//!
//! @file LairdAdvFormat.h
//!
//! @brief Format Specifiers for the Manufacturer Specific Data Type
//!
//! Copyright (c) 2020 Laird Connectivity
//! All Rights Reserved.
/******************************************************************************/

#ifndef LAIRD_ADV_FORMAT_H
#define LAIRD_ADV_FORMAT_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <stdint.h>

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

#define LAIRD_CONNECTIVITY 0x0077
#define LAIRD_COMPANY_ID2 0x00E4

#define LAIRD_ADV_PROTOCOL_ID_RESERVED 0x0000
#define LAIRD_ADV_PROTOCOL_ID_SENTRIUS 0x0001
#define LAIRD_ADV_PROTOCOL_ID_SENTRIUS_EXTENDED 0x0002
#define LAIRD_ADV_PROTOCOL_ID_SENTRIUS_SCAN_RSP 0x0003

#define LAIRD_PRODUCT_ID_RESERVED 0x0000
#define LAIRD_PRODUCT_ID 0x0000

#define LAIRD_ADV_FORMAT_HW_VERSION(major, minor) ((uint8_t)(((((uint32_t)(major)) << 3) & 0x000000F8) | \
                                                   ((((uint32_t)(minor)) << 0 ) & 0x00000007))

#define LAIRD_ADV_PROTOCOL_FIRMWARE_TYPE_BT510 0x00

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif

// end
