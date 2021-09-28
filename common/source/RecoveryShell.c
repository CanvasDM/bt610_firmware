/**
 * @file RecoveryShell.c
 * @brief
 *
 * Copyright (c) 2021 Laird Connectivity
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
#include <tinycrypt/sha256.h>

#include "Attribute.h"
#include "file_system_utilities.h"
#include "FrameworkIncludes.h"

/******************************************************************************/
// Local Constant, Macro and Type Definitions
/******************************************************************************/
#define CHANGE_TEXT_START_POSITION 3
#define REC13_START_CHARACTER 'A'
#define REC13_END_CHARACTER 'F'
#define REC13_ADDITION 13
#define NUMBER_TO_LETTER_ADDITION ('a' - '0')
#define NUMBER_TO_LETTER_START_CHARACTER '0'
#define NUMBER_TO_LETTER_END_CHARACTER '9'
#define RECOVERY_CODE_SHIFT_LETTERS 3
#define RECOVERY_CODE_SHA256_ROUNDS 3
#define RECOVERY_CODE_PREFIX "CONF"
#define RECOVERY_CODE_SUFFIX "_LC_ALLOW_%02d_GO"
#define RECOVERY_CODE_INVALID_SLEEP_TIME K_SECONDS(3)
#define RECOVERY_BUFFER_SIZE 32
#define RECOVERY_ERROR_STRING_SIZE 32
#define REQUEST_CODE_PREFIX "rec"
#define SHA256_ASCII_NULL_SIZE (TC_SHA256_DIGEST_SIZE * 2) + 1
#define SHA256_ASCII_SIZE (TC_SHA256_DIGEST_SIZE * 2)
#ifndef TC_CRYPTO_SUCCESS
#define TC_CRYPTO_SUCCESS 1
#endif

#define HEX_TO_ASCII_CHAR_WIDTH 2
#define HEX_TO_ASCII_A_INPUT 0xf0
#define HEX_TO_ASCII_B_INPUT 0x0f
#define HEX_TO_ASCII_A_INPUT_SHIFT 4
#define HEX_TO_ASCII_B_POSITION 1

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static void hex_to_ascii(uint8_t *out, uint8_t *in, size_t length,
			 bool null_terminate);
static int rec_request_cmd(const struct shell *shell, size_t argc, char **argv);
static int rec_perform_cmd(const struct shell *shell, size_t argc, char **argv);

static void recovery_work_handler(struct k_work *item);
static int recovery_shell_init(const struct device *device);

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
static struct k_work recovery_work;

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
SHELL_STATIC_SUBCMD_SET_CREATE(sub_recovery,
			       SHELL_CMD(request, NULL, "Request recovery code",
					 rec_request_cmd),
			       SHELL_CMD(perform, NULL,
					 "Perform recovery using <code>",
					 rec_perform_cmd),
			       SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(recovery, &sub_recovery, "Module recovery", NULL);

SYS_INIT(recovery_shell_init, POST_KERNEL, 0);

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
static void hex_to_ascii(uint8_t *out, uint8_t *in, size_t length,
			 bool null_terminate)
{
	uint8_t i = 0;
	static uint8_t mapping[] = { '0', '1', '2', '3', '4', '5', '6', '7',
				     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	while (i < length) {
		out[(i * HEX_TO_ASCII_CHAR_WIDTH)] =
			mapping[((in[i] & HEX_TO_ASCII_A_INPUT) >>
				 HEX_TO_ASCII_A_INPUT_SHIFT)];
		out[((i * HEX_TO_ASCII_CHAR_WIDTH) + HEX_TO_ASCII_B_POSITION)] =
			mapping[(in[i] & HEX_TO_ASCII_B_INPUT)];
		++i;
	}

	if (null_terminate) {
		out[(i * HEX_TO_ASCII_CHAR_WIDTH)] = 0;
	}
}

static int rec_request_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r;
	uint8_t buffer[RECOVERY_BUFFER_SIZE];
	uint8_t recover_settings_count = 0;

	/* Create a recovery code which consists of a prefix, the BLE address
	 * and the number of times the unit has been recovered
	 */
	sprintf(buffer, REQUEST_CODE_PREFIX);
	r = Attribute_Get(ATTR_INDEX_bluetoothAddress, &buffer[strlen(buffer)],
			  sizeof(buffer) - strlen(buffer));
	if (r > 0) {
		r = Attribute_Get(ATTR_INDEX_recoverSettingsCount,
				  &recover_settings_count,
				  sizeof(recover_settings_count));
	}

	if (r > 0) {
		sprintf(&buffer[strlen(buffer)], "%02x",
			recover_settings_count);

		uint8_t i = CHANGE_TEXT_START_POSITION;
		uint8_t l = strlen(buffer);
		while (i < l) {
			if (buffer[i] >= REC13_START_CHARACTER &&
			    buffer[i] <= REC13_END_CHARACTER) {
				buffer[i] += REC13_ADDITION;
			} else if (buffer[i] >=
					   NUMBER_TO_LETTER_START_CHARACTER &&
				   buffer[i] <=
					   NUMBER_TO_LETTER_END_CHARACTER) {
				buffer[i] += NUMBER_TO_LETTER_ADDITION;
			}

			++i;
		}

		shell_print(shell, "Recovery request code: %s", buffer);
		r = 0;
	} else {
		shell_error(shell, "Internal error");
		r = -EIO;
	}

	return r;
}

static int rec_perform_cmd(const struct shell *shell, size_t argc, char **argv)
{
	int r = 0;
	uint8_t buffer[ATTR_MAX_STR_SIZE];
	uint8_t recover_settings_count = 0;
	uint8_t sha256hash[TC_SHA256_DIGEST_SIZE];
	uint8_t sha256ascii[SHA256_ASCII_NULL_SIZE];
	uint8_t error_string[RECOVERY_ERROR_STRING_SIZE];
	struct tc_sha256_state_struct sha256data;

	if ((argc == 2) && (argv[1] != NULL)) {
		if (strlen(argv[1]) != SHA256_ASCII_SIZE) {
			sprintf(error_string, "Invalid parameter size");
			r = -EINVAL;
		} else {
			/* Prepare the string which is used for the recovery
			 * verification code
			 */
			sprintf(buffer, RECOVERY_CODE_PREFIX);
			r = Attribute_Get(ATTR_INDEX_bluetoothAddress,
					  &buffer[strlen(buffer)],
					  sizeof(buffer) - strlen(buffer));
			if (r > 0) {
				r = Attribute_Get(
					ATTR_INDEX_recoverSettingsCount,
					&recover_settings_count,
					sizeof(recover_settings_count));
			}

			if (r <= 0) {
				sprintf(error_string, "Error processing");
				r = -EINVAL;
			}

			if (r > 0) {
				sprintf(&buffer[strlen(buffer)],
					RECOVERY_CODE_SUFFIX,
					recover_settings_count);

				/* Shift all characters by 3 */
				uint8_t i = 0;
				uint8_t l = strlen(buffer);
				while (i < l) {
					buffer[i] +=
						RECOVERY_CODE_SHIFT_LETTERS;
					++i;
				}

				/* Hash data once with SHA256 */
				r = tc_sha256_init(&sha256data);

				if (r == TC_CRYPTO_SUCCESS) {
					r = tc_sha256_update(&sha256data,
							     buffer, l);
				}

				if (r == TC_CRYPTO_SUCCESS) {
					r = tc_sha256_final(sha256hash,
							    &sha256data);
				}

				/* Re-hash data twice more with SHA256 for a
				 * total of 3 rounds
				 */
				i = 1;
				while (i < RECOVERY_CODE_SHA256_ROUNDS &&
				       r == TC_CRYPTO_SUCCESS) {
					memcpy(buffer, sha256hash,
					       sizeof(sha256hash));
					r = tc_sha256_init(&sha256data);

					if (r == TC_CRYPTO_SUCCESS) {
						r = tc_sha256_update(
							&sha256data, buffer,
							sizeof(sha256hash));
					}

					if (r == TC_CRYPTO_SUCCESS) {
						r = tc_sha256_final(sha256hash,
								    &sha256data);
					}

					++i;
				}

				if (r != TC_CRYPTO_SUCCESS) {
					sprintf(error_string,
						"Internal completion error");
					r = -EIO;
				} else {
					/* Convert to ASCII */
					hex_to_ascii(sha256ascii, sha256hash,
						     sizeof(sha256hash), true);

					/* Check if this matches the input
					 * provided by the user
					 */
					if (strcmp(sha256ascii, argv[1]) == 0) {
						k_work_submit(&recovery_work);
						shell_print(
							shell,
							"Beginning module recovery");
						r = 0;
					} else {
						/* Invalid code, sleep for 3
						 * seconds before returning
						 */
						k_sleep(RECOVERY_CODE_INVALID_SLEEP_TIME);
						sprintf(error_string,
							"Provided code is not valid");
						r = -EINVAL;
					}
				}
			}
		}
	} else {
		sprintf(error_string, "Unexpected parameters");
		r = -EINVAL;
	}

	if (r != 0) {
		shell_error(shell, "%s", error_string);
	}

	return r;
}

static void recovery_work_handler(struct k_work *item)
{
	ARG_UNUSED(item);
	int r;
	uint8_t recover_settings_count = 0;

	/* Increment recovery code and save to filesystem */
	r = Attribute_Get(ATTR_INDEX_recoverSettingsCount,
			  &recover_settings_count,
			  sizeof(recover_settings_count));
	++recover_settings_count;
	Attribute_SetUint32(ATTR_INDEX_recoverSettingsCount,
			    recover_settings_count);
	fsu_write_abs(CONFIG_RECOVERY_FILE_PATH, &recover_settings_count,
		      sizeof(recover_settings_count));

	/* Begin factory restore of defaults */
	FRAMEWORK_MSG_CREATE_AND_SEND(FWK_ID_CONTROL_TASK, FWK_ID_CONTROL_TASK,
				      FMC_FACTORY_RESET);
}

static int recovery_shell_init(const struct device *device)
{
	ARG_UNUSED(device);
	k_work_init(&recovery_work, recovery_work_handler);
	return 0;
}
