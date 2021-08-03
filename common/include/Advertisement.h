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
typedef struct {
	SensorEvent_t event;
	uint32_t id;
} SensorMsg_t;

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
int Advertisement_Update(SensorMsg_t *sensor_event);
int Advertisement_IntervalUpdate(void);
void SetPasskey(void);
void TestEventMsg(uint16_t event);
bool GetPairingFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADVERTISEMENT_H__ */
