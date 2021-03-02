/* main.c - Application main entry point */

/*
 * Copyright (c) 2019 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "FrameworkIncludes.h"
#include "ControlTask.h"
#include "BspSupport.h"
#include <device.h>
#include <power/power.h>

#define CONSOLE_LABEL DT_LABEL(DT_CHOSEN(zephyr_console))
/* Prevent deep sleep (system off) from being entered on long timeouts
 * or `K_FOREVER` due to the default residency policy.
 *
 * This has to be done before anything tries to sleep, which means
 * before the threading system starts up between PRE_KERNEL_2 and
 * POST_KERNEL.  Do it at the start of PRE_KERNEL_2.
 */
static int disable_ds_1(const struct device *dev)
{
	ARG_UNUSED(dev);

	sys_pm_ctrl_disable_state(SYS_POWER_STATE_DEEP_SLEEP_1);
	return 0;
}
SYS_INIT(disable_ds_1, PRE_KERNEL_2, 0);
void main(void)
{
	//int rc;
	//const struct device *cons = device_get_binding(CONSOLE_LABEL);
	BSP_Init();
	Framework_Initialize();
	//rc = device_set_power_state(cons, DEVICE_PM_ACTIVE_STATE, NULL, NULL);

	ControlTask_Initialize();
	ControlTask_Thread();

}
