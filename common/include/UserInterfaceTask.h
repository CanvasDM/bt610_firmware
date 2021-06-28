/**
 * @file UserInterfaceTask.h
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __USER_INTERFACE_TASK_H__
#define __USER_INTERFACE_TASK_H__

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/
/**
 * @brief
 */
void UserInterfaceTask_Initialize(void);

/**
 * @brief Run LED test
 *
 * @param duration of each step in milliseconds
 *
 * @retval negative error code, 0 on success
 */
int UserInterfaceTask_LedTest(uint32_t duration);

/**
 * @brief Updates the status of mag switch simulation.
 *
 * @param [in]simulation_enabled - The new simulation status.
 * @param [in]last_simulation_enabled - The previous simulation status.
 * @return 0 or greater on success.
 */
int UserInterfaceTask_UpdateMagSwitchSimulatedStatus(
	bool simulation_enabled, bool last_simulation_enabled);

/**
 * @brief Updates the mag switch simulated value.
 *
 * @param [in]simulated_value - The new simulation value.
 * @param [in]last_simulated_value - The previous simulated value.
 * @return 0 or greater on success.
 */
int UserInterfaceTask_UpdateMagSwitchSimulatedValue(bool simulated_value,
						    bool last_simulated_value);

/**
 * @brief Updates the status of tamper switch simulation.
 *
 * @param [in]simulation_enabled - The new simulation status.
 * @param [in]last_simulation_enabled - The previous simulation status.
 * @return 0 or greater on success.
 */
int UserInterfaceTask_UpdateTamperSwitchSimulatedStatus(
	bool simulation_enabled, bool last_simulation_enabled);

/**
 * @brief Updates the tamper switch simulated value.
 *
 * @param [in]simulated_value - The new simulation value.
 * @param [in]last_simulated_value - The previous simulated value.
 * @return 0 or greater on success.
 */
int UserInterfaceTask_UpdateTamperSwitchSimulatedValue(
	bool simulated_value, bool last_simulated_value);

#ifdef __cplusplus
}
#endif

#endif /* __USER_INTERFACE_TASK_H__ */