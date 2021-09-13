/**
 * @file AttributeValidator.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(attrval, LOG_LEVEL_DBG);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <string.h>
#include <ctype.h>

#include "AnalogInput.h"
#include "AttributeTable.h"
#include "BspSupport.h"
#include "Attribute.h"
#include "UserInterfaceTask.h"

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
extern AttributeEntry_t attrTable[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int validate_analog_input_config(void);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int AttributeValidator_string(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	int r = -EPERM;

	/* -1 to account for NULL */
	if (pEntry->size > Length) {
		/* Don't use strncmp because pValue may not be NUL terminated */
		size_t currentLength = strlen(pEntry->pData);
		if (DoWrite && ((memcmp(pEntry->pData, pValue, Length) != 0) ||
				Length == 0 || currentLength != Length)) {
			pEntry->modified = true;
			memset(pEntry->pData, 0, pEntry->size);
			strncpy(pEntry->pData, pValue, Length);
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_uint64(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	uint64_t value = *(uint64_t *)pValue;

	if (DoWrite && value != *((uint64_t *)pEntry->pData)) {
		pEntry->modified = true;
		*((uint64_t *)pEntry->pData) = value;
	}

	return 0;
}

int AttributeValidator_uint32(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value;
	memcpy(&value, pValue, sizeof(value));

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite && value != *((uint32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint32_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_uint16(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint16_t *)pValue);

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite && value != *((uint16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint16_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_bool(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	/* Same as UINT8 */
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint8_t *)pValue);

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite && value != *((uint8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint8_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_uint8(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint8_t *)pValue);

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite && value != *((uint8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((uint8_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_int64(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int64_t value = *(int64_t *)pValue;

	if (DoWrite && value != *((int64_t *)pEntry->pData)) {
		pEntry->modified = true;
		*((int64_t *)pEntry->pData) = value;
	}

	return 0;
}

int AttributeValidator_int32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = *(int32_t *)pValue;

	if (((value >= pEntry->min.sx) && (value <= pEntry->max.sx)) ||
	    (pEntry->min.sx == pEntry->max.sx)) {
		if (DoWrite && value != *((int32_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int32_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_int16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = (int32_t)(*(int16_t *)pValue);

	if (((value >= pEntry->min.sx) && (value <= pEntry->max.sx)) ||
	    (pEntry->min.sx == pEntry->max.sx)) {
		if (DoWrite && value != *((int16_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int16_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_int8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = (int32_t)(*(int8_t *)pValue);

	if (((value >= pEntry->min.sx) && (value <= pEntry->max.sx)) ||
	    (pEntry->min.sx == pEntry->max.sx)) {
		if (DoWrite && value != *((int8_t *)pEntry->pData)) {
			pEntry->modified = true;
			*((int8_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_float(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	float value = *((float *)pValue);

	if (((value >= pEntry->min.fx) && (value <= pEntry->max.fx)) ||
	    (pEntry->min.fx == pEntry->max.fx)) {
		if (DoWrite && value != *((float *)pEntry->pData)) {
			pEntry->modified = true;
			*((float *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_aic(AttributeEntry_t *pEntry, void *pValue,
			   size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint8_t saved = *((uint8_t *)pEntry->pData);
	r = AttributeValidator_uint8(pEntry, pValue, Length, false);
	if (r == 0) {
		/* Assume value is ok.  This makes secondary validation simpler
		 * because it is independent of the channel being changed.
		 */
		*((uint8_t *)pEntry->pData) = *(uint8_t *)pValue;

		r = validate_analog_input_config();
		if (r < 0 || !DoWrite) {
			*((uint8_t *)pEntry->pData) = saved;
			pEntry->modified = false;
		} else {
			pEntry->modified = true;
		}
	}

	if (r < 0) {
		LOG_ERR("Invalid analog input configuration");
	}

	return r;
}
/**
 * @brief Control Point Validators
 * Don't check if value is the same because this is a control point.
 */
int AttributeValidator_cp32(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = *(uint32_t *)pValue;

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite) {
			pEntry->modified = true;
			*((uint32_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_cp16(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint16_t *)pValue);

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite) {
			pEntry->modified = true;
			*((uint16_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_cp8(AttributeEntry_t *pEntry, void *pValue,
			   size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint8_t *)pValue);

	if (((value >= pEntry->min.ux) && (value <= pEntry->max.ux)) ||
	    (pEntry->min.ux == pEntry->max.ux)) {
		if (DoWrite) {
			pEntry->modified = true;
			*((uint8_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_cpi32(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = *(int32_t *)pValue;

	if (((value >= pEntry->min.sx) && (value <= pEntry->max.sx)) ||
	    (pEntry->min.sx == pEntry->max.sx)) {
		if (DoWrite) {
			pEntry->modified = true;
			*((int32_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_cpi16(AttributeEntry_t *pEntry, void *pValue,
			     size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = (int32_t)(*(int16_t *)pValue);

	if (((value >= pEntry->min.sx) && (value <= pEntry->max.sx)) ||
	    (pEntry->min.sx == pEntry->max.sx)) {
		if (DoWrite) {
			pEntry->modified = true;
			*((int16_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_cpi8(AttributeEntry_t *pEntry, void *pValue,
			    size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	int32_t value = (int32_t)(*(int8_t *)pValue);

	if (((value >= pEntry->min.sx) && (value <= pEntry->max.sx)) ||
	    (pEntry->min.sx == pEntry->max.sx)) {
		if (DoWrite) {
			pEntry->modified = true;
			*((int8_t *)pEntry->pData) = value;
		}
		r = 0;
	}

	return r;
}

int AttributeValidator_din1simen(AttributeEntry_t *pEntry, void *pValue,
				 size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint8_t initial_input_state;
	bool enable_state;
	bool last_enable_state;
	bool start_input_state;
	AttributeEntry_t *attribute_entry;

	/* If DoWrite is set, data has been validated and we can
	 * perform updates.
	 */
	if (DoWrite) {
		/* Get the current input state for use later.
		 * If we're changing to enabled, we apply this to the simulated
		 * value to avoid getting any glitches on the input.
		 */
		attribute_entry = &attrTable[ATTR_INDEX_digitalInput];
		initial_input_state = *((uint8_t *)(attribute_entry->pData));
		/* And the current simulation state. We need to know
		 * this so we can determine if simulation is being enabled
		 * If so we need to set the initial state of the simulated
		 * digital input so it matches the live state.
		 */
		attribute_entry = &attrTable[ATTR_INDEX_digitalInput1Simulated];
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* And the state requested to switch to. */
		enable_state = *((bool *)(pValue));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* Now set the initial simulated state */
			start_input_state =
				(bool)(initial_input_state & DIN1_PIN_MASK);
			/* Do this via lowest level access so we don't
			 * trigger the validator.
			 */
			attribute_entry =
				&attrTable[ATTR_INDEX_digitalInput1SimulatedValue];
			*((bool *)(attribute_entry->pData)) = start_input_state;
		}
		/* Update the Digital Input simulation status - note the order
		 * how we do things here is important depending upon whether
		 * simulation is being enabled or disabled.
                 */
		attribute_entry = &attrTable[ATTR_INDEX_digitalInput1Simulated];
		if (enable_state) {
			/* If we're enabling simulation, we set the enable
			 * flag last so we can store the last live input state
			 */
			if ((r = BSP_UpdateDigitalInput1SimulatedStatus(
				     *((bool *)(pValue)), last_enable_state)) >=
			    0) {
				*((bool *)(attribute_entry->pData)) =
					enable_state;
			}
		} else {
			/* If we're disabling simulation, we clear the enable
			 * flag first so we can read the live input state
			 */
			*((bool *)(attribute_entry->pData)) = enable_state;
			r = BSP_UpdateDigitalInput1SimulatedStatus(
				*((bool *)(pValue)), last_enable_state);
		}
	} else {
		/* If DoWrite is not set, this is the first validator call
		 * used to check the validity of the data being written.
		 * In this case, just call the standard validator for the
		 * simulation enable type.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return r;
}

int AttributeValidator_din1sim(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r;
	bool last_simulated_state;
	AttributeEntry_t *attribute_entry;

	/* If DoWrite is set, the data has already been validated */
	if (DoWrite) {
		/* Get the current simulated state for use later */
		attribute_entry =
			&attrTable[ATTR_INDEX_digitalInput1SimulatedValue];
		last_simulated_state = *((bool *)(attribute_entry->pData));
		/* Set the simulated value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pValue));
		/* And update the system */
		r = BSP_UpdateDigitalInput1SimulatedValue(
			     *((bool *)(pValue)), last_simulated_state);
	} else {
		/* If DoWrite is not set, this is the initial validator
		 * call to validate the data passed by the client. We
		 * can call the standard type validator in this case.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return r;
}

int AttributeValidator_din2simen(AttributeEntry_t *pEntry, void *pValue,
				 size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r = -EPERM;
	uint8_t initial_input_state;
	bool enable_state;
	bool last_enable_state;
	bool start_input_state;
	AttributeEntry_t *attribute_entry;

	/* If DoWrite is set, data has been validated and we can
	 * perform updates.
	 */
	if (DoWrite) {
		/* Get the current input state for use later.
		 * If we're changing to enabled, we apply this to the simulated
		 * value to avoid getting any glitches on the input.
		 */
		attribute_entry = &attrTable[ATTR_INDEX_digitalInput];
		initial_input_state = *((uint8_t *)(attribute_entry->pData));
		/* And the current simulation state. We need to know
		 * this so we can determine if we need to set the
		 * initial state of the simulated digital input.
		 */
		attribute_entry = &attrTable[ATTR_INDEX_digitalInput2Simulated];
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* And the state requested to switch to. */
		enable_state = *((bool *)(pValue));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* Now set the initial simulated state */
			start_input_state =
				(bool)(initial_input_state & DIN2_PIN_MASK);
			/* Do this via lowest level access so we don't
			 * trigger the validator.
			 */
			attribute_entry =
				&attrTable[ATTR_INDEX_digitalInput2SimulatedValue];
			*((bool *)(attribute_entry->pData)) = start_input_state;
		}
		/* Update the Digital Input simulation status so we can
		 * determine or perform the behaviour needed when simulation is
		 * disabled.
                 */
		attribute_entry = &attrTable[ATTR_INDEX_digitalInput2Simulated];
		if (enable_state) {
			/* If we're enabling simulation, we set the enable
			 * flag last so we can store the last live input state
			 */
			if ((r = BSP_UpdateDigitalInput2SimulatedStatus(
				     *((bool *)(pValue)), last_enable_state)) >=
			    0) {
				*((bool *)(attribute_entry->pData)) =
					enable_state;
			}
		} else {
			/* If we're disabling simulation, we clear the enable
			 * flag first so we can read the live input state
			 */
			*((bool *)(attribute_entry->pData)) = enable_state;
			r = BSP_UpdateDigitalInput2SimulatedStatus(
				*((bool *)(pValue)), last_enable_state);
		}
	} else {
		/* If DoWrite is not set, this is the first validator call
		 * used to check the validity of the data being written.
		 * In this case, just call the standard validator for the
		 * simulation enable type.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return r;
}

int AttributeValidator_din2sim(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite)
{
	ARG_UNUSED(Length);
	int r;
	bool last_simulated_state;
	AttributeEntry_t *attribute_entry;

	/* If DoWrite is set, the data has already been validated */
	if (DoWrite) {
		/* Get the current simulated state for use later */
		attribute_entry =
			&attrTable[ATTR_INDEX_digitalInput2SimulatedValue];
		last_simulated_state = *((bool *)(attribute_entry->pData));
		/* Set the simulated value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pValue));
		/* Then update the system */
		r = BSP_UpdateDigitalInput2SimulatedValue(
			     *((bool *)(pValue)), last_simulated_state);
	} else {
		/* Call standard validator for initial call */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return r;
}

int AttributeValidator_magsimen(AttributeEntry_t *pEntry, void *pValue,
				size_t Length, bool DoWrite)
{
	int r = 0;
	bool last_enable_state;
	bool enable_state;
	AttributeEntry_t *attribute_entry;
	bool initial_switch_state;

	/* If DoWrite is set, this is the second call of this function so we
	 * can go ahead and apply changes
	 */
	if (DoWrite) {
		/* Get the current simulation state so we can check for a
		 * change in status.
		 */
		attribute_entry = &attrTable[ATTR_INDEX_magSwitchSimulated];
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* Also the state requested to switch to */
		enable_state = *((bool *)(pValue));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* If so, store the live value to avoid glitching */
			attribute_entry = &attrTable[ATTR_INDEX_magnetState];
			initial_switch_state =
				*((bool *)(attribute_entry->pData));
			attribute_entry =
				&attrTable[ATTR_INDEX_magSwitchSimulatedValue];
			*((bool *)(attribute_entry->pData)) =
				initial_switch_state;
		}
		/* Update simulation status. */
		attribute_entry = &attrTable[ATTR_INDEX_magSwitchSimulated];
		/* If simulation is being enabled we update the status first so
		 * we can store the live switch value.
		 */
		if (enable_state) {
			UserInterfaceTask_UpdateMagSwitchSimulatedStatus(
				enable_state, last_enable_state);
			*((bool *)(attribute_entry->pData)) = enable_state;
		} else {
			/* If simulation is being disabled, we update the
			 * status last so the live switch value can be read.
			 */
			*((bool *)(attribute_entry->pData)) = enable_state;
			UserInterfaceTask_UpdateMagSwitchSimulatedStatus(
				enable_state, last_enable_state);
		}
	} else {
		/* If DoWrite is not set, this is the first call so we just
		 * call the standard type validator to check its content.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return (r);
}

int AttributeValidator_magsim(AttributeEntry_t *pEntry, void *pValue,
			      size_t Length, bool DoWrite)
{
	int r = 0;
	bool last_simulated_value;
	AttributeEntry_t *attribute_entry;

	/* If DoWrite is set the data has been validated */
	if (DoWrite) {
		/* Get the current simulated value */
		attribute_entry =
			&attrTable[ATTR_INDEX_magSwitchSimulatedValue];
		last_simulated_value = *((bool *)(attribute_entry->pData));
		/* Update the data value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pValue));
		/* Update the value in the User Interface */
		r = UserInterfaceTask_UpdateMagSwitchSimulatedValue(
			     *((bool *)(pValue)), last_simulated_value);
	} else {
		/* If DoWrite is not set, we just call the
		 * standard type validator to make sure the
		 * data is OK.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return (r);
}

int AttributeValidator_tampsimen(AttributeEntry_t *pEntry, void *pValue,
				 size_t Length, bool DoWrite)
{
	int r = 0;
	bool last_enable_state;
	bool enable_state;
	AttributeEntry_t *attribute_entry;
	bool initial_switch_state;

	/* If DoWrite is set, this is the second call of this function so we
	 * can go ahead and apply changes
	 */
	if (DoWrite) {
		/* Get the current simulation state so we can check for a
		 * change in status.
		 */
		attribute_entry = &attrTable[ATTR_INDEX_tamperSwitchSimulated];
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* Also the state requested to switch to */
		enable_state = *((bool *)(pValue));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* If so, store the live value to avoid glitching */
			attribute_entry =
				&attrTable[ATTR_INDEX_tamperSwitchStatus];
			initial_switch_state =
				*((bool *)(attribute_entry->pData));
			attribute_entry =
				&attrTable[ATTR_INDEX_tamperSwitchSimulatedValue];
			*((bool *)(attribute_entry->pData)) =
				initial_switch_state;
		}
		/* Update simulation status. */
		attribute_entry = &attrTable[ATTR_INDEX_tamperSwitchSimulated];
		/* If simulation is being enabled we update the status first so
		 * we can store the live switch value.
		 */
		if (enable_state) {
			UserInterfaceTask_UpdateTamperSwitchSimulatedStatus(
				enable_state, last_enable_state);
			*((bool *)(attribute_entry->pData)) = enable_state;
		} else {
			/* If simulation is being disabled, we update the
			 * status last so the live switch value can be read.
			 */
			*((bool *)(attribute_entry->pData)) = enable_state;
			UserInterfaceTask_UpdateTamperSwitchSimulatedStatus(
				enable_state, last_enable_state);
		}
	} else {
		/* If DoWrite is not set, this is the first call so we just
		 * call the standard type validator to check its content.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return (r);
}

int AttributeValidator_tampsim(AttributeEntry_t *pEntry, void *pValue,
			       size_t Length, bool DoWrite)
{
	int r = 0;
	bool last_simulated_value;
	AttributeEntry_t *attribute_entry;

	/* If DoWrite is set the data has been validated */
	if (DoWrite) {
		/* Get the current simulated value */
		attribute_entry =
			&attrTable[ATTR_INDEX_tamperSwitchSimulatedValue];
		last_simulated_value = *((bool *)(attribute_entry->pData));
		/* Update the data value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pValue));
		/* Then the value in the User Interface */
		r = UserInterfaceTask_UpdateTamperSwitchSimulatedValue(
			     *((bool *)(pValue)), last_simulated_value);
	} else {
		/* If DoWrite is not set, we just call the
		 * standard type validator to make sure the
		 * data is OK.
		 */
		r = AttributeValidator_bool(pEntry, pValue, Length, false);
	}

	return (r);
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static int validate_analog_input_config(void)
{
	int pressure_sensors = 0;
	int ultrasonic_sensors = 0;
	uint32_t ch;
	size_t i;

	/* This assumes the 4 channels have consecutive IDs. */
	for (i = 0; i < ANALOG_INPUT_NUMBER_OF_CHANNELS; i++) {
		ch = 0;
		memcpy(&ch, attrTable[ATTR_INDEX_analogInput1Type + i].pData,
		       attrTable[ATTR_INDEX_analogInput1Type + i].size);

		switch (ch) {
		case ANALOG_PRESSURE:
			pressure_sensors += 1;
			break;
		case ANALOG_ULTRASONIC:
			ultrasonic_sensors += 1;
			break;
		default:
			/* There aren't any restrictions on the number of voltage or
			 * current sense inputs.
			 */
			break;
		}
	}

	if (ultrasonic_sensors > ANALOG_INPUTS_MAX_ULTRASONIC ||
	    pressure_sensors > ANALOG_INPUTS_MAX_PRESSURE_SENSORS) {
		return -EPERM;
	} else if (ultrasonic_sensors > 0 &&
		   (pressure_sensors >
		    ANALOG_INPUTS_MAX_PRESSURE_SENSORS_WITH_ULTRASONIC)) {
		return -EPERM;
	} else {
		return 0;
	}
}
