/**
 * @file AggregationCount.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(AggregationCount, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>

#include "Version.h"
#include "lcz_sensor_adv_format.h"
#include "lcz_sensor_event.h"
#include "laird_bluetooth.h"
#include "Attribute.h"
#include "AggregationCount.h"
#include "SensorTask.h"
#include "Flags.h"

/******************************************************************************/
/* Local Constant, Macro and Type Definitions                                 */
/******************************************************************************/
#define AGGREGATION_MAX_SIZE (32)

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
float temperatureBuffer[TOTAL_THERM_CH][AGGREGATION_MAX_SIZE];
float analogBuffer[TOTAL_ANALOG_CH][AGGREGATION_MAX_SIZE];
/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AggregationTempHandler(size_t channel)
{
	float currentTemp = 0;
	uint8_t aggCount = 0;
	static uint8_t currentAggregationNumber = 0;
	int r = -EPERM;

	r = Attribute_Get(ATTR_INDEX_AggregationCount, &aggCount, sizeof(aggCount));
	r = Attribute_GetFloat(&currentTemp,
			       ATTR_INDEX_temperatureResult1 + channel);
	if (r == 0) {
		if (aggCount > 1) {
			if (currentAggregationNumber <= aggCount) {
				temperatureBuffer[channel]
						 [currentAggregationNumber] =
							 currentTemp;
				currentAggregationNumber =
					currentAggregationNumber + 1;
			}
			if (currentAggregationNumber == aggCount) {
				/* TODO: Send to event LOG */
			}
		} else {
			/* TODO: Send to event LOG */
		}
	}
	return r;
}

int AggregationAnalogHandler(size_t channel)
{
	float currentAnalogValue = 0;
	uint8_t aggCount = 0;
	static uint8_t currentAggregationNumber = 0;
	int r = -EPERM;

	r = Attribute_Get(ATTR_INDEX_AggregationCount, &aggCount, sizeof(aggCount));
	r = Attribute_GetFloat(&currentAnalogValue,
			       ATTR_INDEX_analogInput1 + channel);
	if (r == 0) {
		if (aggCount > 1) {
			if (currentAggregationNumber <= aggCount) {
				analogBuffer[channel][currentAggregationNumber] =
					currentAnalogValue;
				currentAggregationNumber =
					currentAggregationNumber + 1;
			}
			if (currentAggregationNumber == aggCount) {
				/* TODO: Send to event LOG */
			}
		} else {
			/* TODO: Send to event LOG */
		}
	}
	return r;
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/

/******************************************************************************/
/* Interrupt Service Routines                                                 */
/******************************************************************************/
