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
 * @brief Setup of the different advertisment PHY types
 */
int Advertisement_Init(void);

/**
 * @brief Stops the device from advertising 
 */
int Advertisement_End(void);

/**
 * @brief Starts advertising with the PHY that is enabled
 */
int Advertisement_Start(void);

/**
 * @brief Will force the the coded PHY(Extended advertisment) to begin 
 *  when status is set to true
 *
 * @param status This will enable or disable the coded PHY
 */
void Advertisement_ExtendedSet(bool status);

/**
 * @brief This function will update bytes that have changed in advertisment
 *
 * @param sensor_event There are fields in the advert for events, and this is that data
 *
 * @retval negative error code, 0 on success
 */
int Advertisement_Update(SensorMsg_t *sensor_event);

/**
 * @brief The advertisment interval has been set elsewhere and needs 
 * parameter changed and read from the attribute
 *
 * @retval negative error code, 0 on success
 */
int Advertisement_IntervalUpdate(void);

/**
 * @brief This will set the interval back to what default was and 
 * not change interval attribute
 *
 * @retval negative error code, 0 on success
 */
int Advertisement_IntervalDefault(void);

/**
 * @brief This will be used on init to make sure the passkey is loaded 
 * from memory
 */
void SetPasskey(void);
/**
 * @brief This is only used for the test menue to create a simulated event
 *
 * @param status event number that will be created
 */
void TestEventMsg(uint16_t event);

/**
 * @brief Read the current status of the flag
 *
 * @retval current value of the pairing flag
 */
bool GetPairingFlag(void);

#ifdef __cplusplus
}
#endif

#endif /* __ADVERTISEMENT_H__ */
