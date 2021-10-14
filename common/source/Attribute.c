/**
 * @file Attribute.c
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(attr, CONFIG_ATTR_LOG_LEVEL);

/******************************************************************************/
/* Includes                                                                   */
/******************************************************************************/
#include <zephyr.h>
#include <init.h>
#include <stdio.h>
#include <logging/log_ctrl.h>
#include <sys/util.h>
#include <sys/crc.h>

#include "lcz_param_file.h"
#include "file_system_utilities.h"
#include "AttributeTable.h"
#include "Attribute.h"
#include "ControlTask.h"
#include "NonInit.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define BREAK_ON_ERROR(x)                                                      \
	if (x < 0) {                                                           \
		break;                                                         \
	}

#define TAKE_MUTEX(m) k_mutex_lock(&m, K_FOREVER)
#define GIVE_MUTEX(m) k_mutex_unlock(&m)

#define ATTR_ABS_PATH                                                          \
	CONFIG_LCZ_PARAM_FILE_MOUNT_POINT "/" CONFIG_ATTR_FILE_NAME

#define ATTR_QUIET_ABS_PATH CONFIG_FSU_MOUNT_POINT "/quiet.bin"

static const char EMPTY_STRING[] = "";

/* Each feedback file entry consists of a 2 character id, an equals to sign
 * character, a two character error code and a new line character.
 */
#define ATTR_LOAD_FEEDBACK_ENTRY_SIZE 6

/* Malloc always wants to align on a 4 byte boundary */
#define ATTR_LOAD_FEEDBACK_ALIGN_SIZE 4

/* Time (in ms) between checks if a save is currently in progress */
#define ATTR_SAVE_EXECUTING_CHECK_TIME_MS 200

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
K_MUTEX_DEFINE(attr_mutex);
K_MUTEX_DEFINE(attr_work_mutex);
K_MUTEX_DEFINE(attr_save_change_mutex);

extern AttributeEntry_t attrTable[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#ifdef CONFIG_ATTR_SHELL
static struct k_work workShow;
#endif

static struct k_work_delayable attr_save_delayed_work;

static bool quiet[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int SaveAndBroadcast(attr_idx_t Index);
static int SaveAttributes(bool Immediately);
static void Broadcast(void);

static int LoadAttributes(const char *fname, const char *feedback_path,
			  bool ValidateFirst, bool MaskModified,
			  bool SkipNonWriteable);

static int Loader(param_kvp_t *kvp, char *fstr, size_t pairs, bool DoWrite,
		  bool MaskModified, uint16_t *error_count,
		  bool SkipNonWriteable);

static int Validate(attr_idx_t Index, AttrType_t Type, void *pValue,
		    size_t Length);

static int Write(attr_idx_t Index, AttrType_t Type, void *pValue,
		 size_t Length);

extern void AttributeTable_Initialize(void);
extern void AttributeTable_FactoryReset(void);

static void Show(attr_idx_t Index);

static param_t ConvertParameterType(attr_idx_t idx);
static size_t GetParameterLength(attr_idx_t idx);

static int PrepareForRead(attr_idx_t Index);
static bool isValid(attr_idx_t Index);
static bool isReadable(attr_idx_t Index);
static bool isWritable(attr_idx_t Index);
static bool isDumpRw(attr_idx_t Index);
static bool isDumpW(attr_idx_t Index);
static bool isDumpRo(attr_idx_t Index);

static int InitializeQuiet(void);

static AttrWriteError_T DiagnoseParameterWriteError(param_kvp_t *kvp);

static AttrWriteError_T DiagnoseNumericParameterWriteError(param_kvp_t *kvp,
							   AttrType_t Type);

static AttrWriteError_T DiagnoseStringParameterWriteError(param_kvp_t *kvp);

static int BuildFeedbackFile(const char *feedback_path, char *fstr,
			     param_kvp_t *kvp, size_t pairs);

static int BuildEmptyFeedbackFile(const char *feedback_path);

#ifdef CONFIG_ATTR_SHELL
static void systemWorkqShowHandler(struct k_work *item);
#endif

static void SaveAttributesWork(struct k_work *item);

/******************************************************************************/
/* Global Function Definitions                                                */
/******************************************************************************/
int Attribute_Init(void)
{
	int r = -EPERM;

	TAKE_MUTEX(attr_mutex);

	AttributeTable_Initialize();

	if (fsu_get_file_size_abs(ATTR_ABS_PATH) < 0) {
		r = 0;
		LOG_INF("Parameter file doesn't exist");
	} else {
		r = LoadAttributes(ATTR_ABS_PATH, NULL, false, true, false);
	}

#ifdef CONFIG_ATTR_SHELL
	k_work_init(&workShow, systemWorkqShowHandler);
#endif

	k_work_init_delayable(&attr_save_delayed_work, SaveAttributesWork);

	InitializeQuiet();

	GIVE_MUTEX(attr_mutex);

	return r;
}

int Attribute_FactoryReset(void)
{
	AttributeTable_FactoryReset();
	return SaveAttributes(true);
}

AttrType_t Attribute_GetType(attr_idx_t Index)
{
	if (isValid(Index)) {
		return attrTable[Index].type;
	} else {
		return ATTR_TYPE_UNKNOWN;
	}
}

bool Attribute_ValidIndex(attr_idx_t Index)
{
	return isValid(Index);
}

int Attribute_Set(attr_idx_t Index, AttrType_t Type, void *pValue,
		  size_t ValueLength, bool *modified)
{
	int r = -EPERM;

	if (isValid(Index)) {
		if (isWritable(Index)) {
			TAKE_MUTEX(attr_mutex);
			r = Validate(Index, Type, pValue, ValueLength);
			if (r == 0) {
				r = Write(Index, Type, pValue, ValueLength);
				if (modified != NULL) {
					*modified = attrTable[Index].modified;
				}

				if (r == 0) {
					r = SaveAndBroadcast(Index);
				}
			}
			GIVE_MUTEX(attr_mutex);
		}
	}
	return r;
}

int Attribute_GetDefault(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	memset(pValue, 0, ValueLength);
	size_t size;
	int r = -EPERM;

	if (isValid(Index)) {
		size = MIN(attrTable[Index].size, ValueLength);
		memcpy(pValue, attrTable[Index].pDefault, size);
		r = size;
	}
	return r;
}

int Attribute_Get(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	memset(pValue, 0, ValueLength);
	size_t size;
	int r = -EPERM;

	if (isValid(Index)) {
		if (isReadable(Index)) {
			r = PrepareForRead(Index);
			if (r >= 0) {
				TAKE_MUTEX(attr_mutex);
				size = MIN(attrTable[Index].size, ValueLength);
				memcpy(pValue, attrTable[Index].pData, size);
				r = size;
				GIVE_MUTEX(attr_mutex);
			}
		}
	}
	return r;
}

int Attribute_SetString(attr_idx_t Index, char const *pValue,
			size_t ValueLength)
{
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_STRING, (void *)pValue, ValueLength);
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_GetString(char *pValue, attr_idx_t Index, size_t MaxStringLength)
{
	int r = -EPERM;

	if (isValid(Index)) {
		r = PrepareForRead(Index);
		if (r >= 0) {
			strncpy(pValue, attrTable[Index].pData,
				MaxStringLength);
			r = 0;
		}
	}
	return r;
}

int Attribute_SetUint64(attr_idx_t Index, uint64_t Value)
{
	uint64_t local = Value;
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_U64, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_SetSigned64(attr_idx_t Index, int64_t Value)
{
	int64_t local = Value;
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_S64, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_SetUint32(attr_idx_t Index, uint32_t Value)
{
	uint32_t local = Value;
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_SetSigned32(attr_idx_t Index, int32_t Value)
{
	int32_t local = Value;
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_SetFloat(attr_idx_t Index, float Value)
{
	float local = Value;
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_FLOAT, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_SetNoBroadcastUint32(attr_idx_t Index, uint32_t Value)
{
	uint32_t local = Value;
	int r = -EPERM;
	AttributeEntry_t *p = &attrTable[Index];

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		p->modified = false;
		/* Need to save right away function is used by items that can not
		 * operate correctly if data is lost if a reset occurs.
		 */
		if (r == 0) {
			if (p->savable && !p->deprecated) {
				r = SaveAttributes(true);
			}
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;
	AttrType_t type = attrTable[Index].type;

	if (isValid(Index)) {
		if (type == ATTR_TYPE_U32 || type == ATTR_TYPE_U16 ||
		    type == ATTR_TYPE_U8 || type == ATTR_TYPE_BOOL) {
			r = PrepareForRead(Index);
			if (r >= 0) {
				TAKE_MUTEX(attr_mutex);
				memcpy(pValue, attrTable[Index].pData,
				       attrTable[Index].size);
				r = 0;
				GIVE_MUTEX(attr_mutex);
			}
		}
	}
	return r;
}

int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;
	void *pData = attrTable[Index].pData;
	AttrType_t type = attrTable[Index].type;

	if (isValid(Index)) {
		if (type == ATTR_TYPE_S32 || type == ATTR_TYPE_S16 ||
		    type == ATTR_TYPE_S8 || type == ATTR_TYPE_BOOL) {
			r = PrepareForRead(Index);
			if (r >= 0) {
				TAKE_MUTEX(attr_mutex);
				/* sign extend */
				switch (attrTable[Index].size) {
				case 1:
					*pValue = (int32_t)(*((int8_t *)pData));
					break;
				case 2:
					*pValue =
						(int32_t)(*((int16_t *)pData));
					break;
				default:
					*pValue = *(int32_t *)pData;
					break;
				}
				r = 0;
				GIVE_MUTEX(attr_mutex);
			}
		}
	}
	return r;
}

int Attribute_GetFloat(float *pValue, attr_idx_t Index)
{
	*pValue = 0.0;
	int r = -EPERM;

	if (isValid(Index)) {
		if (attrTable[Index].type == ATTR_TYPE_FLOAT) {
			r = PrepareForRead(Index);
			if (r >= 0) {
				TAKE_MUTEX(attr_mutex);
				*pValue = *((float *)attrTable[Index].pData);
				r = 0;
				GIVE_MUTEX(attr_mutex);
			}
		}
	}
	return r;
}

uint32_t Attribute_AltGetUint32(attr_idx_t Index, uint32_t Default)
{
	uint32_t v = Default;
	int r = Attribute_GetUint32(&v, Index);

	if (r != 0) {
		v = Default;
	}
	return v;
}

int32_t Attribute_AltGetSigned32(attr_idx_t Index, int32_t Default)
{
	int32_t v;
	int r = Attribute_GetSigned32(&v, Index);

	if (r != 0) {
		v = Default;
	}
	return v;
}

float Attribute_AltGetFloat(attr_idx_t Index, float Default)
{
	float v;
	int r = Attribute_GetFloat(&v, Index);

	if (r != 0) {
		v = Default;
	}
	return v;
}

const char *Attribute_GetName(attr_idx_t Index)
{
	const char *p = EMPTY_STRING;
	if (isValid(Index)) {
		p = (const char *)attrTable[Index].name;
	}
	return p;
}

size_t Attribute_GetSize(attr_idx_t Index)
{
	size_t size = 0;
	if (isValid(Index)) {
		size = attrTable[Index].size;
	}
	return size;
}

int Attribute_SetMask32(attr_idx_t Index, uint8_t Bit, uint8_t Value)
{
	uint32_t local;
	int r = -EPERM;

	if (isValid(Index) && Bit < 32) {
		TAKE_MUTEX(attr_mutex);
		local = *(uint32_t *)attrTable[Index].pData;
		WRITE_BIT(local, Bit, Value);
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

int Attribute_SetMask64(attr_idx_t Index, uint8_t Bit, uint8_t Value)
{
	uint64_t local;
	int r = -EPERM;

	if (isValid(Index) && Bit < 64) {
		TAKE_MUTEX(attr_mutex);
		local = *(uint64_t *)attrTable[Index].pData;
		local = Value ? (local | BIT64(Bit)) : (local & ~BIT64(Bit));
		r = Write(Index, ATTR_TYPE_ANY, &local, sizeof(local));
		if (r == 0) {
			r = SaveAndBroadcast(Index);
		}
		GIVE_MUTEX(attr_mutex);
	}
	return r;
}

#ifdef CONFIG_ATTR_SHELL

attr_idx_t Attribute_GetIndex(const char *Name)
{
	attr_idx_t i;

	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (strcmp(Name, attrTable[i].name) == 0) {
			break;
		}
	}
	return i;
}

int Attribute_Show(attr_idx_t Index)
{
	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		Show(Index);
		GIVE_MUTEX(attr_mutex);
		return 0;
	} else {
		return -EINVAL;
	}
}

int Attribute_ShowAll(void)
{
	TAKE_MUTEX(attr_work_mutex);
	k_work_submit(&workShow);
	return 0;
}

#endif /* CONFIG_ATTR_SHELL */

int Attribute_Dump(char **fstr, AttrDumpType_t Type)
{
	int r = 0;
	/* When successful, this is the number of parameters added to the
	   dump file
	*/
	int count = 0;
	bool (*dumpable)(attr_idx_t) = isDumpRw;
	attr_idx_t i;

	switch (Type) {
	case ATTR_DUMP_W:
		dumpable = isDumpW;
		break;
	case ATTR_DUMP_RO:
		dumpable = isDumpRo;
		break;
	default:
		dumpable = isDumpRw;
		break;
	}

	/* Update all parameters that need to be prepared before reserving
	   the mutex - we want to avoid stacking up reservations of it,
	   and if an attribute value changes after this, it will still be
	   up to date
	*/
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		if (dumpable(i)) {
			(void)PrepareForRead(i);
		}
	}
	/* Only proceed if prepares were executed OK */
	if (r == 0) {
		TAKE_MUTEX(attr_mutex);

		do {
			for (i = 0; i < ATTR_TABLE_SIZE; i++) {
				if (dumpable(i)) {
					r = lcz_param_file_generate_file(
						i, ConvertParameterType(i),
						attrTable[i].pData,
						GetParameterLength(i), fstr);
					if (r < 0) {
						break;
					} else {
						count += 1;
					}
				}
			}
			/* Don't validate the file if any attribute errors
			   occurred. Break here to jump out the while(0) 
			   loop
			*/
			BREAK_ON_ERROR(r);

			r = lcz_param_file_validate_file(*fstr, strlen(*fstr));

		} while (0);

		GIVE_MUTEX(attr_mutex);
	}
	if (r < 0) {
		k_free(fstr);
		LOG_ERR("Error converting attribute table into file");
	}
	return (r < 0) ? r : count;
}

int Attribute_SetQuiet(attr_idx_t Index, bool Value)
{
	int r = -EPERM;

	if (isValid(Index)) {
		if (quiet[Index] != Value) {
			quiet[Index] = Value;
			r = fsu_write_abs(ATTR_QUIET_ABS_PATH, quiet,
					  sizeof(quiet));
		} else {
			r = 0;
		}
	}
	return r;
}

int Attribute_Load(const char *abs_path, const char *feedback_path,
		   bool *modified)
{
	int r = -EPERM;

	if (modified != NULL) {
		*modified = false;
	}

	TAKE_MUTEX(attr_mutex);
	do {
		r = LoadAttributes(abs_path, feedback_path, true, false, true);
		BREAK_ON_ERROR(r);

		if (modified != NULL) {
			/* See if any attributes were modified prior to save */
			uint16_t i = 0;
			while (i < ATTR_TABLE_SIZE) {
				if (attrTable[i].modified) {
					*modified = true;
					break;
				}

				++i;
			}
		}

		r = SaveAttributes(true);
		BREAK_ON_ERROR(r);

		Broadcast();

	} while (0);
	GIVE_MUTEX(attr_mutex);

	return r;
}

int Attribute_Save_Now(void)
{
	return SaveAttributes(true);
}

bool Attribute_CodedEnableCheck(void)
{
	bool codedPhySelected;
	Attribute_Get(ATTR_INDEX_use_coded_phy, &codedPhySelected,
		      sizeof(codedPhySelected));
	return (codedPhySelected);
}

bool Attribute_IsLocked(void)
{
	bool locked = true;

	uint8_t lock_status =
		  (*((uint8_t *)attrTable[ATTR_INDEX_lock_status].pData));

	if (lock_status == LOCK_STATUS_NOT_SETUP ||
	    lock_status == LOCK_STATUS_SETUP_DISENGAGED) {
		locked = false;
	}

	return locked;
}

void Attribute_UpdateConfig(void)
{
	uint8_t config_version;

	if (Attribute_Get(ATTR_INDEX_config_version, &config_version,
			  sizeof(config_version)) == sizeof(config_version)) {
		config_version++;
		(void)Attribute_SetUint32(ATTR_INDEX_config_version,
					  (uint32_t)config_version);
	}
}

/******************************************************************************/
/* Local Function Definitions                                                 */
/******************************************************************************/
/* Convert the attribute type into the parameter file type */
static param_t ConvertParameterType(attr_idx_t idx)
{
	if (attrTable[idx].type == ATTR_TYPE_STRING) {
		return PARAM_STR;
	} else {
		return PARAM_BIN;
	}
}

static size_t GetParameterLength(attr_idx_t idx)
{
	if (attrTable[idx].type == ATTR_TYPE_STRING) {
		return strlen(attrTable[idx].pData);
	} else {
		return attrTable[idx].size;
	}
}

static int SaveAndBroadcast(attr_idx_t Index)
{
	int r = 0;
	AttributeEntry_t *p = &attrTable[Index];

	if (p->modified) {
		if (p->savable && !p->deprecated) {
			r = SaveAttributes(false);
		}

		Broadcast();
	}
	return r;
}

static int SaveAttributes(bool Immediately)
{
	int32_t r = 0;
	int rc = 0;

	non_init_set_save_flag(true);

	if (Immediately == true) {
		/* Flag set to immediate save, because the state of the device
		 * is unknown and could be about to reboot, cancel a pending
		 * run and do not reschedule another run, just run it in the
		 * function directly
		 */
		if (k_work_delayable_is_pending(&attr_save_delayed_work) ==
		    true) {
			(void)k_work_cancel_delayable(&attr_save_delayed_work);
		}

		SaveAttributesWork(NULL);

		/* We can return the status with an immediate run */
		rc = Attribute_GetSigned32(&r, ATTR_INDEX_attr_save_error_code);
		if (rc == 0) {
			rc = (int)r;
		}
	} else {
		while (k_work_delayable_busy_get(&attr_save_delayed_work) ==
		       true) {
			/* The save task is currently running, yield by
			 * sleeping until it has finished
			 */
			k_sleep(K_MSEC(ATTR_SAVE_EXECUTING_CHECK_TIME_MS));
		}

		/* Schedule task for saving the data */
		if (k_work_delayable_is_pending(&attr_save_delayed_work) ==
		    false) {
			k_work_reschedule(&attr_save_delayed_work,
					  K_MSEC(CONFIG_ATTR_SAVE_DELAY_MS));
		}
	}

	return rc;
}

static void SaveAttributesWork(struct k_work *item)
{
	int r = -EPERM;
	char *fstr = NULL;
	attr_idx_t i;
	uint32_t checksum_file = 0;
	uint32_t checksum_ram = 0;
	uint32_t fstrlen;

	TAKE_MUTEX(attr_save_change_mutex);

	/* Converting to file format is larger, but makes it easier to go between
	 * different versions.
	 */
	do {
		for (i = 0; i < ATTR_TABLE_SIZE; i++) {
			if (attrTable[i].savable && !attrTable[i].deprecated) {
				r = lcz_param_file_generate_file(
					i, ConvertParameterType(i),
					attrTable[i].pData,
					GetParameterLength(i), &fstr);
				if (r < 0) {
					LOG_ERR("Error converting attribute table into file");
					break;
				}
			}
		}
		BREAK_ON_ERROR(r);

		fstrlen = strlen(fstr);
		r = lcz_param_file_validate_file(fstr, fstrlen);
		BREAK_ON_ERROR(r);

		/* Calculate a CRC32 checksum of the current settings file and
		 * pending data in RAM, which avoids an erase and write process
		 * on the storage flash if there are no changes to write
		 */
		r = fsu_crc32_abs(&checksum_file, ATTR_ABS_PATH,
				  fsu_get_file_size_abs(ATTR_ABS_PATH));

		if (r == 0) {
			checksum_ram = crc32_ieee_update(0, fstr, fstrlen);
		} else {
			/* If calculating the checksum fails, skip working out
			 * the checksum of the RAM variables and just set the
			 * checksums to be different
			 */
			checksum_ram = checksum_file + 1;
		}

		if (checksum_file != checksum_ram) {
			/* Checksums mismatch, data has changed, write the
			 * file
			 */
			r = (int)lcz_param_file_write(CONFIG_ATTR_FILE_NAME,
						      fstr, fstrlen);
			LOG_DBG("Wrote %d of %d bytes of parameters to file", r,
				fstrlen);
		}
	} while (0);

	k_free(fstr);

	if (r >= 0) {
		/* Clear unsaved data flag */
		non_init_set_save_flag(false);
	}

	GIVE_MUTEX(attr_save_change_mutex);

	Attribute_SetSigned32(ATTR_INDEX_attr_save_error_code, ((r < 0) ? r : 0));
}

static void Broadcast(void)
{
	size_t msgSize = sizeof(AttrChangedMsg_t);
	AttrChangedMsg_t *pb = BufferPool_Take(msgSize);

	if (pb == NULL) {
		LOG_ERR("Unable to allocate memory for attr broadcast");
	} else {
		pb->header.msgCode = FMC_ATTR_CHANGED;
		pb->header.txId = FWK_ID_RESERVED;
		pb->header.rxId = FWK_ID_RESERVED;

		size_t i;
		for (i = 0; i < ATTR_TABLE_SIZE; i++) {
			if (attrTable[i].modified && attrTable[i].broadcast) {
				pb->list[pb->count++] = (uint8_t)i;
			}

			if (attrTable[i].modified && !quiet[i]) {
				Show(i);
			}

			attrTable[i].modified = false;
		}

		if (pb->count == 0) {
			BufferPool_Free(pb);
		} else if (Framework_Broadcast((FwkMsg_t *)pb, msgSize) !=
			   FWK_SUCCESS) {
			/* no one may have registered for message */
			BufferPool_Free(pb);
		}
	}
}

void Show(attr_idx_t Index)
{
	AttributeEntry_t *p = &attrTable[Index];
	uint32_t u = 0;
	int32_t i = 0;
	uint64_t a = 0;
	int64_t b = 0;
	float f = 0.0;
	char float_str[CONFIG_ATTR_FLOAT_MAX_STR_SIZE];

	/* Passcode needs to be hidden all other IDs can be shown */
	if (Index != ATTR_INDEX_settings_passcode) {
	switch (p->type) {
	case ATTR_TYPE_U8:
	case ATTR_TYPE_U16:
	case ATTR_TYPE_U32:
	case ATTR_TYPE_BOOL:
		memcpy(&u, p->pData, p->size);
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%u", Index, p->name, u);
		break;

	case ATTR_TYPE_S8:
		i = (int32_t)(*(int8_t *)p->pData);
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%d", Index, p->name, i);
		break;

	case ATTR_TYPE_S16:
		i = (int32_t)(*(int16_t *)p->pData);
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%d", Index, p->name, i);
		break;

	case ATTR_TYPE_S32:
		i = *(int32_t *)p->pData;
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%d", Index, p->name, i);
		break;

	case ATTR_TYPE_FLOAT:
		memcpy(&f, p->pData, p->size);
		snprintf(float_str, sizeof(float_str), CONFIG_ATTR_FLOAT_FMT,
			 f);
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%s", Index, p->name,
			log_strdup(float_str));
		break;

	case ATTR_TYPE_STRING:
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "'%s'", Index, p->name,
			log_strdup((char *)p->pData));
		break;

	case ATTR_TYPE_U64:
		a = *(uint64_t *)p->pData;
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%llu", Index, p->name, a);
		break;

	case ATTR_TYPE_S64:
		b = *(int64_t *)p->pData;
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%lld", Index, p->name, b);
		break;

	default:
		LOG_HEXDUMP_DBG(p->pData, p->size, p->name);
		break;
		}
	}
	else
	{
		/* Still show something so the user knows the command did something */
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "%s", Index, p->name, "******");
	}
}

/**
 * @brief Read parameter file from flash and load it into attributes/RAM.
 *
 * @param ValidateFirst Validate entire file when loading from an external
 * source. Otherwise, allow bad pairs when loading from a file that should be good.
 *
 * @param MaskModified Don't set modified flag during initialization
 */
static int LoadAttributes(const char *fname, const char *feedback_path,
			  bool ValidateFirst, bool MaskModified,
			  bool SkipNonWriteable)
{
	int r = -EPERM;
	size_t fsize;
	char *fstr = NULL;
	param_kvp_t *kvp = NULL;
	size_t pairs = 0;
	uint16_t error_count = 0;

	do {
		r = lcz_param_file_parse_from_file(fname, &fsize, &fstr, &kvp);
		LOG_INF("pairs: %d fsize: %d file: %s", r, fsize,
			log_strdup(fname));
		BREAK_ON_ERROR(r);

		pairs = r;

		if (ValidateFirst) {
			r = Loader(kvp, fstr, pairs, false, MaskModified,
				   &error_count, SkipNonWriteable);

			if (error_count != 0) {
				/* Error occured during verification, no point
				 * in continuing
				 */
				r = -EINVAL;
				break;
			}
		}
		BREAK_ON_ERROR(r);

		r = Loader(kvp, fstr, pairs, true, MaskModified, &error_count,
			   SkipNonWriteable);

	} while (0);

	/* If we got as far as building the kvp list and any errors
	 * occurred, build a list of diagnostics for all parameters in
	 * the file.
	 */
	if ((kvp != NULL) && (error_count) && (feedback_path != NULL)) {
		BuildFeedbackFile(feedback_path, fstr, kvp, pairs);
	}
	/* If no errors occurred and we have a feedback path, we just
	 * create an empty file.
	 */
	if ((!error_count) && (feedback_path != NULL)) {
		BuildEmptyFeedbackFile(feedback_path);
	}

	/* Free the kvp allocation only if an allocation has been made */
	if (kvp != NULL) {
		k_free(kvp);
	}
	/* Free the fstr allocation only if an allocation has been made */
	if (fstr != NULL) {
		k_free(fstr);
	}

	LOG_DBG("status %d", r);

	return r;
}

static int Loader(param_kvp_t *kvp, char *fstr, size_t pairs, bool DoWrite,
		  bool MaskModified, uint16_t *error_count,
		  bool SkipNonWriteable)
{
	int r = -EPERM;
	uint8_t bin[ATTR_MAX_HEX_SIZE];
	size_t binlen;
	attr_idx_t i;
	attr_idx_t idx;
	int (*vw)(attr_idx_t, AttrType_t, void *, size_t) =
		DoWrite ? Write : Validate;

	for (i = 0; i < pairs; i++) {
		idx = kvp[i].id;

		if (!isValid(idx)) {
			r = -EPERM;
		} else if (SkipNonWriteable == true && !isWritable(idx)) {
			/* This when importing settings from a remote source,
			 * therefore in the verification mode, skip entries that
			 * are not writeable
			 */
			r = 0;
		} else if (ConvertParameterType(idx) == PARAM_STR) {
			r = vw(idx, ATTR_TYPE_STRING, kvp[i].keystr,
			       kvp[i].length);
		} else {
			/* Attribute validators for numbers don't look at the
			 * length passed into the function.  However, they do
			 * cast based on the size of the parameter.
			 */
			memset(bin, 0, sizeof(bin));

			binlen = hex2bin(kvp[i].keystr, kvp[i].length, bin,
					 sizeof(bin));
			if (binlen <= 0) {
				r = -EINVAL;
				LOG_ERR("Unable to convert hex->bin for idx: %d",
					idx);
			} else {
				r = vw(idx, ATTR_TYPE_ANY, bin, binlen);
			}
		}

		if (r < 0) {
			/* We always update the error count here regardless
			 * of whether break out is enabled.
			 */
			*error_count = *error_count + 1;
			if (IS_ENABLED(CONFIG_ATTR_BREAK_ON_LOAD_FAILURE)) {
				break;
			}
		}

		if (MaskModified) {
			attrTable[idx].modified = false;
		}
	}

	return r;
}

static int Validate(attr_idx_t Index, AttrType_t Type, void *pValue,
		    size_t Length)
{
	int r = -EPERM;
	AttributeEntry_t *p = &attrTable[Index];

	if (Type == p->type || Type == ATTR_TYPE_ANY) {
		r = p->pValidator(p, pValue, Length, false);
	}

	if (r < 0) {
		LOG_WRN("failure %u %s", Index, p->name);
		LOG_HEXDUMP_DBG(pValue, Length, "attr data");
	}
	return r;
}

static int Write(attr_idx_t Index, AttrType_t Type, void *pValue, size_t Length)
{
	int r = -EPERM;
	AttributeEntry_t *p = &attrTable[Index];

	TAKE_MUTEX(attr_save_change_mutex);

	if (Type == p->type || Type == ATTR_TYPE_ANY) {
		r = p->pValidator(p, pValue, Length, true);
	}

	GIVE_MUTEX(attr_save_change_mutex);

	if (r < 0) {
		LOG_WRN("validation failure %u %s", Index, p->name);
		LOG_HEXDUMP_DBG(pValue, Length, "attr data");
	}
	return r;
}

/**
 * @brief Cause actions that will update an attribute.
 * For the majority of attributes, this function doesn't do anything.
 */
static int PrepareForRead(attr_idx_t Index)
{
	int r = 0;
	if (attrTable[Index].pPrepare != NULL) {
		r = attrTable[Index].pPrepare();
	}
	return r;
}

static bool isValid(attr_idx_t Index)
{
	if (Index < ATTR_TABLE_SIZE) {
		return true;
	} else {
		LOG_ERR("Invalid index %u", Index);
		return false;
	}
}

static bool isReadable(attr_idx_t Index)
{
	return attrTable[Index].readable;
}

static bool isWritable(attr_idx_t Index)
{
	bool r = false;
	AttributeEntry_t *p = &attrTable[Index];

	if (p->writable) {
		if (p->lockable) {
			if (Attribute_IsLocked() == false) {
				r = true;
			}
		} else {
			r = true;
		}
	}

	if (!r) {
		LOG_DBG("[%u] %s is Not writable", Index, p->name);
	}
	return r;
}

static bool isDumpRw(attr_idx_t Index)
{
	bool b = false;

	if (isValid(Index)) {
		if (attrTable[Index].readable && !attrTable[Index].donotdump &&
		    !attrTable[Index].deprecated) {
			b = true;
		}
	}

	return b;
}

static bool isDumpW(attr_idx_t Index)
{
	bool b = false;

	if (isValid(Index)) {
		if (attrTable[Index].readable && !attrTable[Index].donotdump &&
		    !attrTable[Index].deprecated && attrTable[Index].writable) {
			b = true;
		}
	}

	return b;
}

static bool isDumpRo(attr_idx_t Index)
{
	bool b = false;

	if (isValid(Index)) {
		if (attrTable[Index].readable && !attrTable[Index].donotdump &&
		    !attrTable[Index].deprecated &&
		    !attrTable[Index].writable) {
			b = true;
		}
	}

	return b;
}

/**
 * @brief Use a file to determine if attribute should be printed by show
 * or made 'quiet'.
 */
static int InitializeQuiet(void)
{
	int r = -EPERM;
	memset(&quiet, false, sizeof(quiet));

	r = fsu_lfs_mount();
	if (r >= 0) {
		r = fsu_read_abs(ATTR_QUIET_ABS_PATH, &quiet, sizeof(quiet));

		/* The quiet file should not be smaller than the number of attrs. */
		if (r != ATTR_TABLE_SIZE) {
			LOG_WRN("Unexpected quiet file size");
			r = -1;
		}

		/* If file doesn't exists, generate default quiet settings. */
		if (r < 0) {
			quiet[ATTR_INDEX_battery_age] = true;
			quiet[ATTR_INDEX_up_time] = true;
			quiet[ATTR_INDEX_qrtc] = true;
			quiet[ATTR_INDEX_security_level] = true;
			r = fsu_write_abs(ATTR_QUIET_ABS_PATH, quiet,
					  sizeof(quiet));

			if (r < 0) {
				LOG_ERR("Unable to write quiet file: %d", r);
			}
		}
	}

	return r;
}

static AttrWriteError_T DiagnoseParameterWriteError(param_kvp_t *kvp)
{
	AttrWriteError_T result = ATTR_WRITE_ERROR_OK;
	AttrType_t attribute_type;

	/* The following apply to all parameter types.
	 * Known parameter index?
	 */
	if (!isValid(kvp->id)) {
		result = ATTR_WRITE_ERROR_PARAMETER_UNKNOWN;
	}
	if (result == ATTR_WRITE_ERROR_OK) {
		/* Writable parameter? */
		if (!isWritable(kvp->id)) {
			result = ATTR_WRITE_ERROR_PARAMETER_READ_ONLY;
		}
	}
	/* If not diagnosed used type specific handlers */
	if (result == ATTR_WRITE_ERROR_OK) {
		attribute_type = Attribute_GetType(kvp->id);
		switch (attribute_type) {
		case (ATTR_TYPE_BOOL):
		case (ATTR_TYPE_U8):
		case (ATTR_TYPE_U16):
		case (ATTR_TYPE_U32):
		case (ATTR_TYPE_U64):
		case (ATTR_TYPE_S8):
		case (ATTR_TYPE_S16):
		case (ATTR_TYPE_S32):
		case (ATTR_TYPE_S64):
		case (ATTR_TYPE_FLOAT):
			result = DiagnoseNumericParameterWriteError(
				kvp, attribute_type);
			break;
		case (ATTR_TYPE_STRING):
			result = DiagnoseStringParameterWriteError(kvp);
			break;
		default:
			break;
		}
	}
	return (result);
}

static AttrWriteError_T DiagnoseNumericParameterWriteError(param_kvp_t *kvp,
							   AttrType_t Type)
{
	AttrWriteError_T result = ATTR_WRITE_ERROR_OK;
	AttributeEntry_t *attribute_entry;
	uint8_t bin[ATTR_MAX_HEX_SIZE];
	size_t binlen;

	attribute_entry = &attrTable[kvp->id];

	/* Get numeric data and length */
	binlen = hex2bin(kvp->keystr, kvp->length, bin, sizeof(bin));
	/* Make sure the data is valid for use */
	if (binlen <= 0) {
		result = ATTR_WRITE_ERROR_PARAMETER_INVALID_LENGTH;
	}
	/* And only proceed if safe to do so */
	if (result == ATTR_WRITE_ERROR_OK) {
		switch (Type) {
		case (ATTR_TYPE_U8):
		case (ATTR_TYPE_BOOL):
			if (*((uint8_t *)(bin)) > attribute_entry->max.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((uint8_t *)(bin)) <
				   attribute_entry->min.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_U16):
			if (*((uint16_t *)(bin)) > attribute_entry->max.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((uint16_t *)(bin)) <
				   attribute_entry->min.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_U32):
			if (*((uint32_t *)(bin)) > attribute_entry->max.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((uint32_t *)(bin)) <
				   attribute_entry->min.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_U64):
			if (*((uint64_t *)(bin)) > attribute_entry->max.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((uint64_t *)(bin)) <
				   attribute_entry->min.ux) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_S8):
			if (*((int8_t *)(bin)) > attribute_entry->max.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((int8_t *)(bin)) <
				   attribute_entry->min.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_S16):
			if (*((int16_t *)(bin)) > attribute_entry->max.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((int16_t *)(bin)) <
				   attribute_entry->min.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_S32):
			if (*((int32_t *)(bin)) > attribute_entry->max.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((int32_t *)(bin)) <
				   attribute_entry->min.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_S64):
			if (*((int64_t *)(bin)) > attribute_entry->max.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((int64_t *)(bin)) <
				   attribute_entry->min.sx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		case (ATTR_TYPE_FLOAT):
			if (*((float *)(bin)) > attribute_entry->max.fx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_HIGH;
			} else if (*((float *)(bin)) <
				   attribute_entry->min.fx) {
				result = ATTR_WRITE_ERROR_NUMERIC_TOO_LOW;
			}
			break;
		default:
			break;
		}
	}
	return (result);
}

static AttrWriteError_T DiagnoseStringParameterWriteError(param_kvp_t *kvp)
{
	AttrWriteError_T result = ATTR_WRITE_ERROR_OK;
	AttributeEntry_t *attribute_entry;

	attribute_entry = &attrTable[kvp->id];

	if (attribute_entry->size < kvp->length) {
		result = ATTR_WRITE_ERROR_STRING_TOO_MANY_CHARACTERS;
	}

	return (result);
}

static int BuildFeedbackFile(const char *feedback_path, char *fstr,
			     param_kvp_t *kvp, size_t pairs)
{
	/* Start with a result greater than zero here
	 * so we enter the while loop
	 */
	int result = 1;
	int pair_index;
	AttrWriteError_T attribute_write_error;
	param_kvp_t *next_kvp;
	uint16_t total_length = 0;
	uint8_t *write_buffer;

	/* Reserve enough space to hold details of all parameters. We add
	 * an extra character here so the append_feedback function can keep
	 * the string buffer null terminated for internal use.
	 */
	uint16_t buffer_size = (ATTR_LOAD_FEEDBACK_ENTRY_SIZE * pairs) + 1;
	/* Also be sure we align properly for malloc calls */
	buffer_size = ((buffer_size / ATTR_LOAD_FEEDBACK_ALIGN_SIZE) *
		       ATTR_LOAD_FEEDBACK_ALIGN_SIZE) +
		      ATTR_LOAD_FEEDBACK_ALIGN_SIZE;

	write_buffer = k_malloc(buffer_size);

	/* Then add our feedback if possible */
	if (write_buffer != NULL) {
		/* Start out with a blank buffer */
		memset(write_buffer, 0x0, buffer_size);
		/* Work through all load KVPs */
		for (pair_index = 0; (pair_index < pairs) && (result > 0);
		     pair_index++) {
			/* Get next KVP for diagnosis */
			next_kvp = &kvp[pair_index];
			/* Diagnose the next parameter error */
			attribute_write_error =
				DiagnoseParameterWriteError(next_kvp);
			/* And add to our output file */
			result = lcz_param_file_append_feedback(
				next_kvp->id, attribute_write_error,
				write_buffer);
			/* Zero and less indicates an error occurred,
			 * greater than zero an added length.
			 */
			if (result > 0) {
				total_length += result;
			}
		}
	}
	/* OK to write our feedback file? */
	if (write_buffer != NULL) {
		/* Any data added? */
		if (total_length) {
			/* Either create or overwrite the feedback file */
			result = fsu_write_abs(feedback_path, write_buffer,
					       total_length);
		}
		/* Free memory regardless */
		k_free(write_buffer);
	}
	return (result);
}

static int BuildEmptyFeedbackFile(const char *feedback_path)
{
	int result;

	result = fsu_write_abs(feedback_path, NULL, 0);

	return (result);
}

/******************************************************************************/
/* System WorkQ context                                                       */
/******************************************************************************/
#ifdef CONFIG_ATTR_SHELL
static void systemWorkqShowHandler(struct k_work *item)
{
	ARG_UNUSED(item);
	attr_idx_t i;

	TAKE_MUTEX(attr_mutex);
	for (i = 0; i < ATTR_TABLE_SIZE; i++) {
		PrepareForRead(i);
		Show(i);
		k_sleep(K_MSEC(CONFIG_ATTR_SHELL_SHOW_ALL_DELAY_MS));
	}
	GIVE_MUTEX(attr_mutex);

	GIVE_MUTEX(attr_work_mutex);
}
#endif
