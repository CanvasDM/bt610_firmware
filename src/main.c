/* main.c - Application main entry point */

/*
 * Copyright (c) 2019-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);
#include <zephyr/zephyr.h>
#include <version.h>
#ifdef CONFIG_ATTR
#include <attr.h>
#endif
#ifdef CONFIG_LCZ_LWM2M_CLIENT
#include <lcz_lwm2m_client.h>
#endif
#ifdef CONFIG_LCZ_LWM2M_FW_UPDATE
#include <lcz_lwm2m_fw_update.h>
#endif
#include "app_version.h"
#ifdef CONFIG_SHELL_BACKEND_SERIAL
#include <zephyr/shell/shell_uart.h>
#endif
#include <zephyr/drivers/uart.h>
#include "ControlTask.h"
#include "BspSupport.h"

/**************************************************************************************************/
/* Local Constant, Macro and Type Definitions                                                     */
/**************************************************************************************************/
#define PKG_NAME_PREFIX "lc_dm"
#define PKG_NAME PKG_NAME_PREFIX "-" CONFIG_BOARD "-"

#define DEFAULT_BAUD_RATE 115200

static struct uart_config uart_cfg = {
	.baudrate = DEFAULT_BAUD_RATE,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.data_bits = UART_CFG_DATA_BITS_8,
	.flow_ctrl = UART_CFG_FLOW_CTRL_RTS_CTS,
};

void main(void)
{
	const struct device *uart_dev;
	int ret;

#ifdef CONFIG_SHELL_BACKEND_SERIAL
	/* Disable log output by default on the UART console.
	 * Re-enable logging using the 'log go' cmd.
	 * LCZ_SHELL_LOGIN selects SHELL_START_OBSCURED which disables log output before main().
	 */
#ifdef CONFIG_ATTR
	if (attr_get_bool(ATTR_ID_log_on_boot) == false)
#endif
	{
		log_backend_deactivate(shell_backend_uart_get_ptr()->log_backend->backend);
	}
#endif

#ifdef CONFIG_ATTR
	if (attr_get_bool(ATTR_ID_disable_flow_control)) {
		uart_cfg.flow_ctrl = UART_CFG_FLOW_CTRL_NONE;
	}
	uart_cfg.baudrate = attr_get_uint32(ATTR_ID_baud_rate, DEFAULT_BAUD_RATE);
#endif

	uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	if (device_is_ready(uart_dev)) {
		ret = uart_configure(uart_dev, &uart_cfg);
		if (ret != 0) {
			LOG_ERR("Could not config console UART [%d]", ret);
		}
	} else {
		LOG_ERR("Console UART not ready");
	}

#ifdef CONFIG_LCZ_LWM2M_CLIENT
	(void)lcz_lwm2m_client_set_device_firmware_version(APP_VERSION_STRING);
	(void)lcz_lwm2m_client_set_software_version(KERNEL_VERSION_STRING);
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
