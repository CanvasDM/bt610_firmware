/**
 * @file FileAccess.c
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(FileAccess, CONFIG_FILE_ACCESS_LOG_LEVEL);
#define THIS_FILE "FileAccess"

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <mgmt/mgmt.h>
#include <lcz_fs_mgmt/fs_mgmt_impl.h>

#include "FileAccess.h"
#include "attr.h"

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int fs_mgmt_impl_app_access_check(uint8_t access_type, const char *path)
{
	int rc = -MGMT_ERR_EPERUSER;

	/* Check if a file is being read or written */
	if (access_type == ACCESS_TYPE_READ) {
		/* Read: allow access to event manager output, parameter dump
		 * output and feedback files only
		 */
		if (strcmp(path, EVENT_MANAGER_FILE_OUT_PATH) == 0) {
			rc = 0;
		} else if (strcmp(path, attr_get_quasi_static(ATTR_ID_dump_path)) ==
			   0) {
			rc = 0;
		} else if (strcmp(path, CONFIG_ATTRIBUTE_MGMT_FEEDBACK_FILE)
			   == 0) {
			rc = 0;
		} else if (strcmp(path, CONFIG_ATTR_SHELL_FEEDBACK_FILE)
			   == 0) {
			rc = 0;
		}
	} else if (access_type == ACCESS_TYPE_WRITE) {
		/* Write: allow access to parameter upload file only */
		if (strcmp(path, attr_get_quasi_static(ATTR_ID_load_path)) == 0) {
			/* But do not allow if the settings lock is engaged */
			if (attr_is_locked() == false) {
				rc = 0;
			}
		}
	}

	LOG_DBG("%s request for %s: %s", (access_type == ACCESS_TYPE_READ
			? "Read" : (access_type == ACCESS_TYPE_WRITE ? "Write"
			: "Unknown")), path, (rc == 0 ? "allowed" : "denied"));

	return rc;
}
