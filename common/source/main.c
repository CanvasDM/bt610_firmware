/* main.c - Application main entry point */

/*
 * Copyright (c) 2019-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
#include <zephyr.h>
#include <version.h>
#ifdef CONFIG_ATTR
#include "attr.h"
#endif
#ifdef CONFIG_LCZ_LWM2M_CLIENT
#include "lcz_lwm2m_client.h"
#endif
#ifdef CONFIG_LCZ_LWM2M_FW_UPDATE
#include "lcz_lwm2m_fw_update.h"
#endif
#include "app_version.h"
#ifdef CONFIG_SHELL_BACKEND_SERIAL
#include <shell/shell_uart.h>
#endif
#include "ControlTask.h"
#include "BspSupport.h"

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define DEFAULT_HW_VERSION "0"
#define PKG_NAME_PREFIX "BT610_DM"
#define PKG_NAME PKG_NAME_PREFIX "[" CONFIG_BOARD "]"

void main(void)
{
#ifdef CONFIG_SHELL_BACKEND_SERIAL
	/* Disable log output by default on the UART console.
	 * Re-enable logging using the 'log go' cmd.
	 * LCZ_SHELL_LOGIN selects SHELL_START_OBSCURED which disables log output before main().
	 */
#ifdef CONFIG_ATTR
	if (*(bool *)attr_get_quasi_static(ATTR_ID_log_on_boot) == false)
#endif
	{
		log_backend_deactivate(shell_backend_uart_get_ptr()->log_backend->backend);
	}
#endif
#ifdef CONFIG_LCZ_LWM2M_CLIENT
	(void)lcz_lwm2m_client_set_device_model_number(CONFIG_BOARD);
	(void)lcz_lwm2m_client_set_device_firmware_version(APP_VERSION_STRING);
	(void)lcz_lwm2m_client_set_software_version(KERNEL_VERSION_STRING);
	(void)lcz_lwm2m_client_set_hardware_version(DEFAULT_HW_VERSION);
#endif
#ifdef CONFIG_LCZ_LWM2M_FW_UPDATE
	(void)lcz_lwm2m_fw_update_set_pkg_name(PKG_NAME);
	(void)lcz_lwm2m_fw_update_set_pkg_version(APP_VERSION_STRING);
#endif
	BSP_Init();
	ControlTask_Initialize();
	ControlTask_Thread();

	LOG_INF("BT610 DM Firmware v%s [%s]", APP_VERSION_STRING, CONFIG_BOARD);
}
