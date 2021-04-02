/**
 * @file Advertisement.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __ADVERTISEMENT_H__
#define __ADVERTISEMENT_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 *
 * @param
 * @param
 *
 * @retval
 */
int Advertisement_Init(void);
int Advertisement_End(void);
int Advertisement_Start(void);
void Advertisement_ExtendedSet(bool status);
int Advertisement_Update(void);
int Advertisement_IntervalUpdate(void);
void SetPasskey(void);
void TestEventMsg(uint16_t event);

#ifdef __cplusplus
}
#endif

#endif /* __ADVERTISEMENT_H__ */
