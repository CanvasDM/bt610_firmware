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
#ifdef CONFIG_SHELL_BACKEND_SERIAL
#include <shell/shell_uart.h>
#endif
#include "ControlTask.h"
#include "BspSupport.h"

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
	BSP_Init();
	ControlTask_Initialize();
	ControlTask_Thread();
}
