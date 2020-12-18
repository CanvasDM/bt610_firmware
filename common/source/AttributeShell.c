/**
 * @file AttributeShell.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
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
#include "file_system_utilities.h"

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int ats_set_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_query_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_dump_cmd(const struct shell *shell, size_t argc, char **argv);
static int ats_show_cmd(const struct shell *shell, size_t argc, char **argv);

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
	SHELL_CMD(dump, NULL,
		  "<0 = all, 1 = rw, 2 = ro> <abs_path>\n"
		  "if path not included then default is /ext/dump.txt",
		  ats_dump_cmd),
	SHELL_CMD(show, NULL, "Display all parameters", ats_show_cmd),
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

	if ((argc == 3) && (argv[1] != NULL) && (argv[2] != NULL)) {
		idx = get_index(argv[1]);
		/* The attribute validators make sense of what is given to them. */
		if (is_string(argv[2])) {
			r = Attribute_Set(idx, ATTR_TYPE_ANY, argv[2],
					  strlen(argv[2]));
		} else {
			long x = strtol(argv[2], NULL, 0);
			r = Attribute_Set(idx, ATTR_TYPE_ANY, &x, 0);
		}
	} else {
		shell_error(shell, "Unexpected parameters");
		return -EINVAL;
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

static int ats_dump_cmd(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	return 0;
}

static int ats_show_cmd(const struct shell *shell, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);
	return 0;
}

static int attr_shell_init(const struct device *device)
{
	ARG_UNUSED(device);
	return 0;
}
