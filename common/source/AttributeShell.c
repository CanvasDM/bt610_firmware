/**
 * @file AttributeShell.c
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <shell/shell.h>
#include <init.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "Attribute.h"
#include "FrameworkIncludes.h"
#include "FileAccess.h"
#include "file_system_utilities.h"
#include "lcz_qrtc.h"

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int ats_set_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_query_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_show_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_qrtc_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_get_cmd(const struct shell *shell, size_t argc, char **argv);
#if defined(CONFIG_ATTR_SHELL_ALLOW_DEBUG_COMMANDS)
static int ats_type_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_dump_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_load_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_quiet_cmd(const struct shell *shell, size_t argc, char **argv);
#endif

static int attr_shell_init(const struct device *device);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(
	sub_attr,
	SHELL_CMD(set, NULL, "set attribute <number or name> <value>",
		  ats_set_cmd),
	SHELL_CMD(query, NULL, "query attribute <number or name>",
		  ats_query_cmd),
	SHELL_CMD(
		get, NULL,
		"get attribute <number or name>\n"
		"If a prepare to read function exists it will be called to update parameter value",
		ats_get_cmd),
	SHELL_CMD(show, NULL, "Display all parameters", ats_show_cmd),
#if defined(CONFIG_ATTR_SHELL_ALLOW_DEBUG_COMMANDS)
	SHELL_CMD(
		dump, NULL,
		"<0 = rw, 1 = w, 2 = ro> <abs_path>\n"
		"if path not included then default is " SENTRIUS_MGMT_PARAMETER_DUMP_PATH,
		ats_dump_cmd),
	SHELL_CMD(
		type, NULL,
		"Display an attribute file\n"
		"<abs file name> <param present then hexdump (default is string)>",
		ats_type_cmd),
	SHELL_CMD(quiet, NULL,
		  "Disable printing for a parameter"
		  "<idx> <0 = verbose, 1= quiet>",
		  ats_quiet_cmd),
	SHELL_CMD(load, NULL, "Load attributes from a file <abs file name>",
		  ats_load_cmd),
#endif
	SHELL_CMD(
		qrtc, NULL,
		"Set the Quasi-RTC (Default is time in seconds from Jan 1, 1970)\n"
		"Value set must be larger than upTime",
		ats_qrtc_cmd),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(attr, &sub_attr, "Attribute (parameter) Utilities", NULL);

SYS_INIT(attr_shell_init, POST_KERNEL, 0);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/

/* Strings can have numbers, but most likely they won't be the first char. */
static bool is_string(const char *str)
{
	if (isdigit((int)str[0])) {
		return false;
	}
	return true;
}

static attr_idx_t get_index(const char *str)
{
	attr_idx_t idx = 0;

	if (is_string(str)) {
		idx = Attribute_GetIndex(str);
	} else {
		idx = (attr_idx_t)strtoul(str, NULL, 0);
	}
	return idx;
}

static int ats_set_cmd(const struct shell *shell, size_t argc, char **argv)
{
	attr_idx_t idx = 0;
	int r = -EPERM;
	AttrType_t type;
	static union {
		uint32_t x;
		int32_t y;
		uint64_t xx;
		int64_t yy;
		bool b;
		float f;
	} param;
	bool modified = false;

	if ((argc == 3) && (argv[1] != NULL) && (argv[2] != NULL)) {
		idx = get_index(argv[1]);

#if defined(CONFIG_SHELL_SELECTIVE_HISTORY)
		if (idx == ATTR_INDEX_settingsPasscode) {
			/* Do not save this command in the shell history */
			shell->history->skip_current_line = true;
		}
#endif

		/* The attribute validators try to make sense of what is given
		 * to them.
		 */
		if (Attribute_ValidIndex(idx)) {
			type = Attribute_GetType(idx);

			switch (type) {
			case ATTR_TYPE_FLOAT:
				param.f = strtof(argv[2], NULL);
				r = Attribute_Set(idx, ATTR_TYPE_ANY, &param.f,
						  sizeof(param.f), &modified);
				break;

			case ATTR_TYPE_BOOL:
			case ATTR_TYPE_U8:
			case ATTR_TYPE_U16:
			case ATTR_TYPE_U32:
				param.x = strtoul(argv[2], NULL, 0);
				r = Attribute_Set(idx, ATTR_TYPE_ANY, &param.x,
						  sizeof(param.x), &modified);
				break;

			case ATTR_TYPE_U64:
				param.xx = strtoull(argv[2], NULL, 0);
				r = Attribute_Set(idx, ATTR_TYPE_ANY, &param.xx,
						  sizeof(param.xx), &modified);
				break;

			case ATTR_TYPE_S8:
			case ATTR_TYPE_S16:
			case ATTR_TYPE_S32:
				param.y = strtol(argv[2], NULL, 0);
				r = Attribute_Set(idx, ATTR_TYPE_ANY, &param.y,
						  sizeof(param.y), &modified);
				break;

			case ATTR_TYPE_S64:
				param.yy = strtoll(argv[2], NULL, 0);
				r = Attribute_Set(idx, ATTR_TYPE_ANY, &param.yy,
						  sizeof(param.yy), &modified);
				break;

			case ATTR_TYPE_STRING:
				r = Attribute_Set(idx, ATTR_TYPE_ANY, argv[2],
						  strlen(argv[2]), &modified);
				break;

			default:
				shell_error(shell, "Unhandled type");
				break;
			}

			if (r < 0) {
				shell_error(shell, "Set failed %d", r);
			}

		} else {
			shell_error(shell, "Invalid id");
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}

	/* Update config version if parameter was modified */
	if (modified) {
		Attribute_UpdateConfig();
	}

	return 0;
}

static int ats_query_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = -EPERM;

	if ((argc == 2) && (argv[1] != NULL)) {
		r = Attribute_Show(get_index(argv[1]));
		shell_print(shell, "query status: %d", r);
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int ats_get_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = -EPERM;
	attr_idx_t idx = 0;
	uint8_t dummy[ATTR_MAX_STR_SIZE];

	if ((argc == 2) && (argv[1] != NULL)) {
		idx = get_index(argv[1]);
		/* Discard data (assumes show is enabled). */
		r = Attribute_Get(idx, dummy, sizeof(dummy));
		/* If the value changed then prepare will cause a duplicate show. */
		Attribute_Show(idx);

		shell_print(shell, "get status: %d", r);
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int ats_show_cmd(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	Attribute_ShowAll();
	return 0;
}

#if defined(CONFIG_ATTR_SHELL_ALLOW_DEBUG_COMMANDS)
static int ats_dump_cmd(const struct shell *shell, size_t argc, char **argv)
{
	char *fstr = NULL;
	char *fname = SENTRIUS_MGMT_PARAMETER_DUMP_PATH;
	int r = -EPERM;
	int type;

	if ((argc >= 2) && (argv[1] != NULL)) {
		type = MAX((int)strtol(argv[1], NULL, 0), 0);
		r = Attribute_Dump(&fstr, type);
		if (r >= 0) {
			shell_print(shell, "Dump status: %d type: %d", r, type);
			if (argc == 3) {
				if (argv[2] != NULL) {
					fname = argv[2];
				}
			}
			r = fsu_write_abs(fname, fstr, strlen(fstr));
		}

		if (r < 0) {
			shell_error(shell, "Dump error %d", r);
		} else {
			shell_print(shell, "%s", fstr);
		}

		k_free(fstr);

	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int ats_type_cmd(const struct shell *shell, size_t argc, char **argv)
{
	char *buf;
	ssize_t size;

	if ((argc >= 2) && (argv[1] != NULL)) {
		size = fsu_get_file_size_abs(argv[1]);
		if (size > 0) {
			buf = k_calloc(size + 1, sizeof(uint8_t));
			fsu_read_abs(argv[1], buf, size);
			if (argc > 2) {
				shell_hexdump(shell, buf, strlen(buf));
			} else {
				shell_print(shell, "%s", buf);
			}
			k_free(buf);
		} else {
			shell_error(shell, "File not found");
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int ats_quiet_cmd(const struct shell *shell, size_t argc, char **argv)
{
	attr_idx_t idx = 0;
	int quiet = 0;
	int r = -EPERM;

	if ((argc == 3) && (argv[1] != NULL) && (argv[2] != NULL)) {
		idx = get_index(argv[1]);
		quiet = MAX((int)strtol(argv[2], NULL, 0), 0);
		r = Attribute_SetQuiet(idx, quiet);
		if (r < 0) {
			shell_error(shell, "Unable to set quiet");
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int ats_load_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = -EPERM;
	if ((argc == 2) && (argv[1] != NULL)) {
		r = Attribute_Load(argv[1],
				   ATTRIBUTE_SHELL_PARAMETER_FEEDBACK_PATH);
		if (r < 0) {
			shell_error(shell, "Attribute Load error");
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}
#endif

static int ats_qrtc_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = -EPERM;
	uint32_t qrtc;
	uint32_t result;

	if ((argc == 2) && (argv[1] != NULL)) {
		qrtc = MAX((int)strtol(argv[1], NULL, 0), 0);
		result = lcz_qrtc_set_epoch(qrtc);
		r = Attribute_SetUint32(ATTR_INDEX_qrtcLastSet, qrtc);
		if (qrtc != result || r < 0) {
			shell_error(shell, "Unable to set qrtc");
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
	}
	return 0;
}

static int attr_shell_init(const struct device *device)
{
	ARG_UNUSED(device);
	return 0;
}
