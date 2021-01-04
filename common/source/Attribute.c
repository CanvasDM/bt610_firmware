/**
 * @file Attribute.c
 * @brief
 *
 * Copyright (c) 2020 Laird Connectivity
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

#include "lcz_params.h"
#include "file_system_utilities.h"

#include "AttributeTable.h"
#include "Attribute.h"

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/
#define BREAK_ON_ERROR(x)                                                      \
	if (x < 0) {                                                           \
		break;                                                         \
	}

#define TAKE_MUTEX(m) k_mutex_lock(&m, K_FOREVER)
#define GIVE_MUTEX(m) k_mutex_unlock(&m)

#define ATTR_ABS_PATH CONFIG_LCZ_PARAMS_MOUNT_POINT "/" CONFIG_ATTR_FILE_NAME

#define ATTR_QUIET_ABS_PATH CONFIG_FSU_MOUNT_POINT "/quiet.bin"

static const char EMPTY_STRING[] = "";

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
K_MUTEX_DEFINE(attr_mutex);
K_MUTEX_DEFINE(attr_work_mutex);

extern AttributeEntry_t attrTable[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Data Definitions                                                     */
/******************************************************************************/
#ifdef CONFIG_ATTR_SHELL
static struct k_work workShow;
#endif

static bool quiet[ATTR_TABLE_SIZE];

/******************************************************************************/
/* Local Function Prototypes                                                  */
/******************************************************************************/
static int SaveAndBroadcast(attr_idx_t Index);
static int SaveAttributes(void);
static void Broadcast(void);

static int LoadAttributes(const char *fname, bool ValidateFirst,
			  bool MaskModified);

static int Loader(param_kvp_t *kvp, char *fstr, size_t pairs, bool DoWrite,
		  bool MaskModified);

static int Validate(attr_idx_t Index, AttrType_t Type, void *pValue,
		    size_t Length);

static int Write(attr_idx_t Index, AttrType_t Type, void *pValue,
		 size_t Length);

extern void AttributeTable_Initialize(void);
extern void AttributeTable_FactoryReset(void);

static void Show(attr_idx_t Index);

static param_t ConvertParameterType(attr_idx_t idx);
static size_t GetParameterLength(attr_idx_t idx);

static bool isValid(attr_idx_t Index);
static bool isWritable(attr_idx_t Index);
static bool isDumpRw(attr_idx_t Index);
static bool isDumpW(attr_idx_t Index);
static bool isDumpRo(attr_idx_t Index);

static int InitializeQuiet(void);

#ifdef CONFIG_ATTR_SHELL
static void systemWorkqShowHandler(struct k_work *item);
#endif

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
		r = LoadAttributes(ATTR_ABS_PATH, false, true);
	}

#ifdef CONFIG_ATTR_SHELL
	k_work_init(&workShow, systemWorkqShowHandler);
#endif

	InitializeQuiet();

	GIVE_MUTEX(attr_mutex);

	return r;
}

int Attribute_FactoryReset(void)
{
	AttributeTable_FactoryReset();
	return SaveAttributes();
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
		  size_t ValueLength, ParamSetLocation_t setLocation)
{
	int r = -EPERM;

	if (isValid(Index)) {
		if (isWritable(Index)) {
			TAKE_MUTEX(attr_mutex);
			r = Validate(Index, Type, pValue, ValueLength);
			if (r == 0) {
				r = Write(Index, Type, pValue, ValueLength);
				if (r == 0) {
					r = SaveAndBroadcast(Index);
				}
			}
			GIVE_MUTEX(attr_mutex);
		}
	}
	return r;
}

int Attribute_Get(attr_idx_t Index, void *pValue, size_t ValueLength)
{
	memset(pValue, 0, ValueLength);
	size_t size = MIN(attrTable[Index].size, ValueLength);
	int r = -EPERM;

	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		memcpy(pValue, attrTable[Index].pData, size);
		r = size;
		GIVE_MUTEX(attr_mutex);
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
		strncpy(pValue, attrTable[Index].pData, MaxStringLength);
		r = 0;
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

int Attribute_GetUint32(uint32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (isValid(Index)) {
		if (attrTable[Index].type == ATTR_TYPE_U32) {
			TAKE_MUTEX(attr_mutex);
			*pValue = *((uint32_t *)attrTable[Index].pData);
			r = 0;
			GIVE_MUTEX(attr_mutex);
		}
	}
	return r;
}

int Attribute_GetSigned32(int32_t *pValue, attr_idx_t Index)
{
	*pValue = 0;
	int r = -EPERM;

	if (isValid(Index)) {
		if (attrTable[Index].type == ATTR_TYPE_S32) {
			TAKE_MUTEX(attr_mutex);
			*pValue = *((int32_t *)attrTable[Index].pData);
			r = 0;
			GIVE_MUTEX(attr_mutex);
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
			TAKE_MUTEX(attr_mutex);
			*pValue = *((float *)attrTable[Index].pData);
			r = 0;
			GIVE_MUTEX(attr_mutex);
		}
	}
	return r;
}

uint32_t Attribute_AltGetUint32(attr_idx_t Index, uint32_t Default)
{
	uint32_t v = Default;
	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		v = 0;
		memcpy(&v, attrTable[Index].pData, attrTable[Index].size);
		GIVE_MUTEX(attr_mutex);
	}
	return v;
}

int32_t Attribute_AltGetSigned32(attr_idx_t Index, int32_t Default)
{
	int32_t v = Default;
	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		v = 0;
		memcpy(&v, attrTable[Index].pData, attrTable[Index].size);
		GIVE_MUTEX(attr_mutex);
	}
	return v;
}

float Attribute_AltGetFloat(attr_idx_t Index, float Default)
{
	float v = Default;
	if (isValid(Index)) {
		TAKE_MUTEX(attr_mutex);
		v = 0.0;
		memcpy(&v, attrTable[Index].pData, attrTable[Index].size);
		GIVE_MUTEX(attr_mutex);
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
	int r = -EPERM;
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

	TAKE_MUTEX(attr_mutex);

	do {
		for (i = 0; i < ATTR_TABLE_SIZE; i++) {
			if (dumpable(i)) {
				r = lcz_params_generate_file(
					i, ConvertParameterType(i),
					attrTable[i].pData,
					GetParameterLength(i), fstr);
				if (r < 0) {
					LOG_ERR("Error converting attribute table into file");
					break;
				} else {
					count += 1;
				}
			}
		}
		BREAK_ON_ERROR(r);

		r = lcz_params_validate_file(*fstr, strlen(*fstr));

	} while (0);

	GIVE_MUTEX(attr_mutex);

	if (r < 0) {
		k_free(fstr);
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

int Attribute_Load(const char *abs_path)
{
	int r = -EPERM;

	TAKE_MUTEX(attr_mutex);
	do {
		r = LoadAttributes(abs_path, true, false);
		BREAK_ON_ERROR(r);

		r = SaveAttributes();
		BREAK_ON_ERROR(r);

		Broadcast();

	} while (0);
	GIVE_MUTEX(attr_mutex);

	return r;
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
			r = SaveAttributes();
		}

		Broadcast();
	}
	return r;
}

static int SaveAttributes(void)
{
	int r = -EPERM;
	char *fstr = NULL;
	attr_idx_t i;

	/* Converting to file format is larger, but makes it easier to go between
	 * different versions.
	 */
	do {
		for (i = 0; i < ATTR_TABLE_SIZE; i++) {
			if (attrTable[i].savable && !attrTable[i].deprecated) {
				r = lcz_params_generate_file(
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

		r = lcz_params_validate_file(fstr, strlen(fstr));
		BREAK_ON_ERROR(r);

		r = (int)lcz_params_write(CONFIG_ATTR_FILE_NAME, fstr,
					  strlen(fstr));
		LOG_DBG("Wrote %d of %d bytes of parameters to file", r,
			strlen(fstr));

	} while (0);

	k_free(fstr);

	return (r < 0) ? r : 0;
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
	uint32_t a = 0;
	uint32_t b = 0;
	float f = 0.0;
	char float_str[CONFIG_ATTR_FLOAT_MAX_STR_SIZE];

	switch (p->type) {
	case ATTR_TYPE_U8:
	case ATTR_TYPE_U16:
	case ATTR_TYPE_U32:
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
	case ATTR_TYPE_S64:
		/* These weren't printing properly */
		memcpy(&a, (uint8_t *)p->pData, 4);
		memcpy(&b, ((uint8_t *)p->pData) + 4, 4);
		LOG_DBG(CONFIG_ATTR_SHOW_FMT "0x%08x %08x", Index, p->name, b,
			a);
		break;

	default:
		LOG_HEXDUMP_DBG(p->pData, p->size, p->name);
		break;
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
static int LoadAttributes(const char *fname, bool ValidateFirst,
			  bool MaskModified)
{
	int r = -EPERM;
	size_t fsize;
	char *fstr = NULL;
	param_kvp_t *kvp = NULL;
	size_t pairs = 0;

	do {
		r = lcz_params_parse_from_file(fname, &fsize, &fstr, &kvp);
		LOG_INF("pairs: %d fsize: %d file: %s", r, fsize,
			log_strdup(fname));
		BREAK_ON_ERROR(r);

		pairs = r;

		if (ValidateFirst) {
			r = Loader(kvp, fstr, pairs, false, MaskModified);
		}
		BREAK_ON_ERROR(r);

		r = Loader(kvp, fstr, pairs, true, MaskModified);

	} while (0);

	k_free(kvp);
	k_free(fstr);

	LOG_DBG("status %d", r);

	return r;
}

static int Loader(param_kvp_t *kvp, char *fstr, size_t pairs, bool DoWrite,
		  bool MaskModified)
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
		} else if (ConvertParameterType(idx) == PARAM_STR) {
			r = vw(idx, ATTR_TYPE_STRING, kvp[i].keystr,
			       kvp[i].length);
		} else {
			/* Attribute validators for numbers don't look at the length passed
			 * into the function.  However, they do cast based on the size
			 * of the parameter.
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

	if (Type == p->type || Type == ATTR_TYPE_ANY) {
		r = p->pValidator(p, pValue, Length, true);
	}

	if (r < 0) {
		LOG_WRN("validation failure %u %s", Index, p->name);
		LOG_HEXDUMP_DBG(pValue, Length, "attr data");
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

static bool isWritable(attr_idx_t Index)
{
	bool r = false;
	bool unlocked = ((*((uint8_t *)attrTable[ATTR_INDEX_lock].pData)) == 0);
	AttributeEntry_t *p = &attrTable[Index];

	if (p->writable) {
		if (p->lockable) {
			r = unlocked;
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
		if (attrTable[Index].readable && !attrTable[Index].deprecated) {
			b = true;
		}
	}

	return b;
}

static bool isDumpW(attr_idx_t Index)
{
	bool b = false;

	if (isValid(Index)) {
		if (attrTable[Index].readable && !attrTable[Index].deprecated &&
		    attrTable[Index].writable) {
			b = true;
		}
	}

	return b;
}

static bool isDumpRo(attr_idx_t Index)
{
	bool b = false;

	if (isValid(Index)) {
		if (attrTable[Index].readable && !attrTable[Index].deprecated &&
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
			LOG_WRN("Unexpected file size");
			r = -1;
		}

		/* If file doesn't exists, generate default quiet settings. */
		if (r < 0) {
			quiet[ATTR_INDEX_batteryAge] = true;
			quiet[ATTR_INDEX_upTime] = true;
			quiet[ATTR_INDEX_qrtc] = true;
			r = fsu_write_abs(ATTR_QUIET_ABS_PATH, quiet,
					  sizeof(quiet));

			if (r < 0) {
				LOG_ERR("Unable to write quiet file: %d", r);
			}
		}
	}

	return r;
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
		Show(i);
		k_sleep(K_MSEC(CONFIG_ATTR_SHELL_SHOW_ALL_DELAY_MS));
	}
	GIVE_MUTEX(attr_mutex);

	GIVE_MUTEX(attr_work_mutex);
}
#endif
