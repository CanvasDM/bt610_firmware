/**
 * @file attr_custom_validator.c
 * @brief
 *
 * Copyright (c) 2022 Laird Connectivity LLC
 *
 * SPDX-License-Identifier: LicenseRef-LairdConnectivity-Clause
 */

/**************************************************************************************************/
/* Includes                                                                                       */
/**************************************************************************************************/
#include <zephyr.h>
#include <device.h>
#include <string.h>
#include <ctype.h>

#include "attr_table.h"
#include "attr_table_private.h"
#include "attr_custom_validator.h"
#include "attr_validator.h"
#include "BspSupport.h"
#include "UserInterfaceTask.h"
#include "SensorTask.h"

#include <logging/log.h>
LOG_MODULE_REGISTER(attrval, CONFIG_ATTR_VALID_LOG_LEVEL);

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define CHECK_ENTRY() __ASSERT(entry != NULL, "Invalid Entry (index)");

/**************************************************************************************************/
/* Global Data Definitions                                                                        */
/**************************************************************************************************/
extern const struct attr_table_entry ATTR_TABLE[ATTR_TABLE_SIZE];

extern atomic_t attr_modified[];

/**************************************************************************************************/
/* Local Function Prototypes                                                                      */
/**************************************************************************************************/
static int validate_analog_input_config(void);

/**************************************************************************************************/
/* Local Function Definitions                                                                     */
/**************************************************************************************************/
static int validate_analog_input_config(void)
{
	int pressure_sensors = 0;
	int ultrasonic_sensors = 0;
	uint32_t ch;
	size_t i;
	const ate_t *attribute_entry;

	/* This assumes the 4 channels have consecutive IDs. */
	for (i = 0; i < ANALOG_INPUT_NUMBER_OF_CHANNELS; i++) {
		attribute_entry = attr_map(ATTR_ID_analog_input_1_type + i);
		ch = 0;
		memcpy(&ch, attribute_entry->pData, attribute_entry->size);

		switch (ch) {
		case ANALOG_INPUT_1_TYPE_PRESSURE:
			pressure_sensors += 1;
			break;
		case ANALOG_INPUT_1_TYPE_ULTRASONIC:
			ultrasonic_sensors += 1;
			break;
		default:
			/* There aren't any restrictions on the number of
			 * voltage or current sense inputs.
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

/**************************************************************************************************/
/* Global Function Definitions                                                                    */
/**************************************************************************************************/
int av_tx_power(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	ARG_UNUSED(vlen);
	CHECK_ENTRY();
	int8_t value = *((int8_t *)pv);
	__ASSERT(entry->size == sizeof(value), "Validator and attribute size mismatch");

	/* Values supported by nRF52840 */
	bool valid = false;
	switch (value) {
	case -40:
	case -20:
	case -16:
	case -12:
	case -8:
	case -4:
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		valid = true;
		break;

	default:
		valid = false;
		break;
	}

	if (valid) {
		if (do_write && value != *((int8_t *)entry->pData)) {
			atomic_set_bit(attr_modified, attr_table_index(entry));
			*((int8_t *)entry->pData) = value;
		}
		return 1;
	}
	return 0;
}

int av_aic(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	ARG_UNUSED(vlen);
	int r = -EPERM;
	uint8_t saved = *((uint8_t *)entry->pData);
	r = av_uint8(entry, pv, vlen, false);
	if (r == 0) {
		/* Assume value is ok. This makes secondary validation simpler
		 * because it is independent of the channel being changed.
		 */
		*((uint8_t *)entry->pData) = *(uint8_t *)pv;

		r = validate_analog_input_config();
		if (r < 0 || !do_write) {
			*((uint8_t *)entry->pData) = saved;
			atomic_clear_bit(attr_modified,
					 attr_table_index(entry));
		} else {
			atomic_set_bit(attr_modified, attr_table_index(entry));
		}
	}

	if (r < 0) {
		LOG_ERR("Invalid analog input configuration");
	}

	return r;
}

int av_din1simen(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	ARG_UNUSED(vlen);
	int r = -EPERM;
	uint8_t initial_input_state;
	bool enable_state;
	bool last_enable_state;
	bool start_input_state;
	const ate_t *attribute_entry;

	/* If do_write is set, data has been validated and we can
	 * perform updates.
	 */
	if (do_write) {
		/* Get the current input state for use later.
		 * If we're changing to enabled, we apply this to the simulated
		 * value to avoid getting any glitches on the input.
		 */
		attribute_entry = attr_map(ATTR_ID_digital_input);
		initial_input_state = *((uint8_t *)(attribute_entry->pData));
		/* And the current simulation state. We need to know
		 * this so we can determine if simulation is being enabled
		 * If so we need to set the initial state of the simulated
		 * digital input so it matches the live state.
		 */
		attribute_entry = attr_map(ATTR_ID_digital_input_1_simulated);
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* And the state requested to switch to. */
		enable_state = *((bool *)(pv));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* Now set the initial simulated state */
			start_input_state = (bool)(initial_input_state &
						   DIGITAL_INPUT_PORT1_BITMASK);
			/* Do this via lowest level access so we don't
			 * trigger the validator.
			 */
			attribute_entry = attr_map(
				ATTR_ID_digital_input_1_simulated_value);
			*((bool *)(attribute_entry->pData)) = start_input_state;
		}
		/* Update the Digital Input simulation status - note the order
		 * how we do things here is important depending upon whether
		 * simulation is being enabled or disabled.
                 */
		attribute_entry = attr_map(ATTR_ID_digital_input_1_simulated);
		if (enable_state) {
			/* If we're enabling simulation, we set the enable
			 * flag last so we can store the last live input state
			 */
			if ((r = BSP_UpdateDigitalInput1SimulatedStatus(
				     *((bool *)(pv)), last_enable_state)) >=
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
				*((bool *)(pv)), last_enable_state);
		}
	} else {
		/* If do_write is not set, this is the first validator call
		 * used to check the validity of the data being written.
		 * In this case, just call the standard validator for the
		 * simulation enable type.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return r;
}

int av_din1sim(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	ARG_UNUSED(vlen);
	int r;
	bool last_simulated_state;
	const ate_t *attribute_entry;

	/* If do_write is set, the data has already been validated */
	if (do_write) {
		/* Get the current simulated state for use later */
		attribute_entry =
			attr_map(ATTR_ID_digital_input_1_simulated_value);
		last_simulated_state = *((bool *)(attribute_entry->pData));
		/* Set the simulated value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pv));
		/* And update the system */
		r = BSP_UpdateDigitalInput1SimulatedValue(*((bool *)(pv)),
							  last_simulated_state);
	} else {
		/* If do_write is not set, this is the initial validator
		 * call to validate the data passed by the client. We
		 * can call the standard type validator in this case.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return r;
}

int av_din2simen(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	ARG_UNUSED(vlen);
	int r = -EPERM;
	uint8_t initial_input_state;
	bool enable_state;
	bool last_enable_state;
	bool start_input_state;
	const ate_t *attribute_entry;

	/* If do_write is set, data has been validated and we can
	 * perform updates.
	 */
	if (do_write) {
		/* Get the current input state for use later.
		 * If we're changing to enabled, we apply this to the simulated
		 * value to avoid getting any glitches on the input.
		 */
		attribute_entry = attr_map(ATTR_ID_digital_input);
		initial_input_state = *((uint8_t *)(attribute_entry->pData));
		/* And the current simulation state. We need to know
		 * this so we can determine if we need to set the
		 * initial state of the simulated digital input.
		 */
		attribute_entry = attr_map(ATTR_ID_digital_input_2_simulated);
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* And the state requested to switch to. */
		enable_state = *((bool *)(pv));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* Now set the initial simulated state */
			start_input_state = (bool)(initial_input_state &
						   DIGITAL_INPUT_PORT2_BITMASK);
			/* Do this via lowest level access so we don't
			 * trigger the validator.
			 */
			attribute_entry = attr_map(
				ATTR_ID_digital_input_2_simulated_value);
			*((bool *)(attribute_entry->pData)) = start_input_state;
		}
		/* Update the Digital Input simulation status so we can
		 * determine or perform the behaviour needed when simulation is
		 * disabled.
                 */
		attribute_entry = attr_map(ATTR_ID_digital_input_2_simulated);
		if (enable_state) {
			/* If we're enabling simulation, we set the enable
			 * flag last so we can store the last live input state
			 */
			if ((r = BSP_UpdateDigitalInput2SimulatedStatus(
				     *((bool *)(pv)), last_enable_state)) >=
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
				*((bool *)(pv)), last_enable_state);
		}
	} else {
		/* If do_write is not set, this is the first validator call
		 * used to check the validity of the data being written.
		 * In this case, just call the standard validator for the
		 * simulation enable type.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return r;
}

int av_din2sim(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	ARG_UNUSED(vlen);
	int r;
	bool last_simulated_state;
	const ate_t *attribute_entry;

	/* If do_write is set, the data has already been validated */
	if (do_write) {
		/* Get the current simulated state for use later */
		attribute_entry =
			attr_map(ATTR_ID_digital_input_2_simulated_value);
		last_simulated_state = *((bool *)(attribute_entry->pData));
		/* Set the simulated value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pv));
		/* Then update the system */
		r = BSP_UpdateDigitalInput2SimulatedValue(*((bool *)(pv)),
							  last_simulated_state);
	} else {
		/* Call standard validator for initial call */
		r = av_bool(entry, pv, vlen, false);
	}

	return r;
}

int av_magsimen(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	int r = 0;
	bool last_enable_state;
	bool enable_state;
	const ate_t *attribute_entry;
	bool initial_switch_state;

	/* If do_write is set, this is the second call of this function so we
	 * can go ahead and apply changes
	 */
	if (do_write) {
		/* Get the current simulation state so we can check for a
		 * change in status.
		 */
		attribute_entry = attr_map(ATTR_ID_mag_switch_simulated);
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* Also the state requested to switch to */
		enable_state = *((bool *)(pv));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* If so, store the live value to avoid glitching */
			attribute_entry = attr_map(ATTR_ID_magnet_state);
			initial_switch_state =
				*((bool *)(attribute_entry->pData));
			attribute_entry =
				attr_map(ATTR_ID_mag_switch_simulated_value);
			*((bool *)(attribute_entry->pData)) =
				initial_switch_state;
		}
		/* Update simulation status. */
		attribute_entry = attr_map(ATTR_ID_mag_switch_simulated);
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
		/* If do_write is not set, this is the first call so we just
		 * call the standard type validator to check its content.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return (r);
}

int av_magsim(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	int r = 0;
	bool last_simulated_value;
	const ate_t *attribute_entry;

	/* If do_write is set the data has been validated */
	if (do_write) {
		/* Get the current simulated value */
		attribute_entry = attr_map(ATTR_ID_mag_switch_simulated_value);
		last_simulated_value = *((bool *)(attribute_entry->pData));
		/* Update the data value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pv));
		/* Update the value in the User Interface */
		r = UserInterfaceTask_UpdateMagSwitchSimulatedValue(
			*((bool *)(pv)), last_simulated_value);
	} else {
		/* If do_write is not set, we just call the
		 * standard type validator to make sure the
		 * data is OK.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return (r);
}

int av_tampsimen(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	int r = 0;
	bool last_enable_state;
	bool enable_state;
	const ate_t *attribute_entry;
	bool initial_switch_state;

	/* If do_write is set, this is the second call of this function so we
	 * can go ahead and apply changes
	 */
	if (do_write) {
		/* Get the current simulation state so we can check for a
		 * change in status.
		 */
		attribute_entry = attr_map(ATTR_ID_tamper_switch_simulated);
		last_enable_state = *((bool *)(attribute_entry->pData));
		/* Also the state requested to switch to */
		enable_state = *((bool *)(pv));
		/* Is simulation being enabled? */
		if ((!last_enable_state) && (enable_state)) {
			/* If so, store the live value to avoid glitching */
			attribute_entry =
				attr_map(ATTR_ID_tamper_switch_status);
			initial_switch_state =
				*((bool *)(attribute_entry->pData));
			attribute_entry =
				attr_map(ATTR_ID_tamper_switch_simulated_value);
			*((bool *)(attribute_entry->pData)) =
				initial_switch_state;
		}
		/* Update simulation status. */
		attribute_entry = attr_map(ATTR_ID_tamper_switch_simulated);
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
		/* If do_write is not set, this is the first call so we just
		 * call the standard type validator to check its content.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return (r);
}

int av_tampsim(const ate_t *const entry, void *pv, size_t vlen, bool do_write)
{
	int r = 0;
	bool last_simulated_value;
	const ate_t *attribute_entry;

	/* If do_write is set the data has been validated */
	if (do_write) {
		/* Get the current simulated value */
		attribute_entry =
			attr_map(ATTR_ID_tamper_switch_simulated_value);
		last_simulated_value = *((bool *)(attribute_entry->pData));
		/* Update the data value */
		*((bool *)(attribute_entry->pData)) = *((bool *)(pv));
		/* Then the value in the User Interface */
		r = UserInterfaceTask_UpdateTamperSwitchSimulatedValue(
			*((bool *)(pv)), last_simulated_value);
	} else {
		/* If do_write is not set, we just call the
		 * standard type validator to make sure the
		 * data is OK.
		 */
		r = av_bool(entry, pv, vlen, false);
	}

	return (r);
}

int av_block_downgrades(const ate_t *const entry, void *pv, size_t vlen,
			bool do_write)
{
	/* Same as UINT8 */
	ARG_UNUSED(vlen);
	int r = -EPERM;
	uint32_t value = (uint32_t)(*(uint8_t *)pv);

	if (((value >= entry->min.ux) && (value <= entry->max.ux)) ||
	    (entry->min.ux == entry->max.ux)) {
		if (value == 1 || value == *((uint8_t *)entry->pData)) {
			if (do_write && value != *((uint8_t *)entry->pData)) {
				atomic_set_bit(attr_modified,
					       attr_table_index(entry));
				*((uint8_t *)entry->pData) = value;
			}
			r = 0;
		} else {
			/* Cannot disable blocking downgrades once enabled */
			r = -EINVAL;
		}
	}

	return r;
}
