/**
 * @file FileAccess.h
 * @brief
 *
 * Copyright (c) 2021-2022 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __FILE_ACCESS_H__
#define __FILE_ACCESS_H__

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************************************/
/* Global Function Prototypes                                                                     */
/**************************************************************************************************/
/**
 * @brief Sets up file access control for mcumgr fs read/write requests
 */
void file_access_setup(void);

#ifdef __cplusplus
}
#endif

#endif /* __FILE_ACCESS_H__ */
