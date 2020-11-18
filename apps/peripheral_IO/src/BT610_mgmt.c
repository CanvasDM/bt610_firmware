/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <limits.h>
#include <string.h>
#include "cborattr/cborattr.h"
#include "mgmt/mgmt.h"
#include "BT610_mgmt.h"
#include "BT610_mgmt_impl.h"
#include "BT610_mgmt_config.h"

static mgmt_handler_fn bt610_mgmt_read;
static mgmt_handler_fn bt610_mgmt_write;

static struct {
	/** Whether an upload is currently in progress. */
	bool uploading;

	/** Expected offset of next upload request. */
	size_t off;

	/** Total length of file currently being uploaded. */
	size_t len;
} fs_mgmt_ctxt;

static const struct mgmt_handler bt610_mgmt_handlers[] = {
	// pystart - mgmt handlers
    [BT610_MGMT_ID_GETALLTEMPERATURE] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE1] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE2] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE3] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE4] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETCURRENT] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETBATTERYVOLTAGE] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETDIGITALINPUTALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE1ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE2ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE3ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE4ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG1ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG2ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG3ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG4ALARMS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETHARDWAREVERSION] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETFIRMWAREVERSION] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETRESETREASON] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETBLUETOOTHMAC] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETBLUETOOTHMTU] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETFLAGS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETRESETCOUNT] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETSENSORNAME] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETSENSORLOCATION] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETBLEADVERTISINGINTERVAL] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETBLEADVERTISINGDURATION] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETSETTINGSLOCK] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETBATTERYSENSEINTERVAL] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURESENSEINTERVAL] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATUREAGGREGATIONVALUE] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETDIGITALOUTPUT1] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETDIGITALOUTPUT2] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETDIGITALINPUT1] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETDIGITALINPUT2] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOGINPUT1TYPE] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE1ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE2ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE3ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTEMPERATURE4ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG1ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG2ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG3ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETANALOG4ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETACTIVEMODE] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETUSECODEDPHY] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETTXPOWER] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETNETWORKID] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETCONFIGVERSION] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_GETMAGNETSTATE] = {
    .mh_read = bt610_mgmt_read
    },
    [BT610_MGMT_ID_SETSENSORNAME] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETSENSORLOCATION] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETBLEADVERTISINGINTERVAL] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETBLEADVERTISINGDURATION] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETBLEPASSKEY] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETSETTINGSLOCK] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETBATTERYSENSEINTERVAL] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTEMPERATURESENSEINTERVAL] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTEMPERATUREAGGREGATIONVALUE] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETDIGITALOUTPUT1] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETDIGITALOUTPUT2] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETDIGITALINPUT1] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETDIGITALINPUT2] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETANALOGINPUTTYPE] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTEMPERATURE1ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTEMPERATURE2ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTEMPERATURE3ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTEMPERATURE4ALARMTHRESHOLD] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETANALOG1ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETANALOG2ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETANALOG3ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETANALOG4ALARMTHRESHOLDS] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETACTIVEMODE] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETUSECODEDPHY] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETTXPOWER] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETNETWORKID] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETCONFIGVERSION] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETHARDWAREVERSION] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
    [BT610_MGMT_ID_SETLEDTEST] = {
    .mh_read = bt610_mgmt_read
    .mh_write = bt610_mgmt_write
    },
	// pyend
};

#define FS_MGMT_HANDLER_CNT                                                    \
	(sizeof fs_mgmt_handlers / sizeof fs_mgmt_handlers[0])

static struct mgmt_group fs_mgmt_group = {
	.mg_handlers = fs_mgmt_handlers,
	.mg_handlers_count = FS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_FS,
};

/**
 * Command handler: fs file (read)
 */
static int fs_mgmt_file_download(struct mgmt_ctxt *ctxt)
{
	uint8_t file_data[FS_MGMT_DL_CHUNK_SIZE];
	char path[FS_MGMT_PATH_SIZE + 1];
	unsigned long long off;
	CborError err;
	size_t bytes_read;
	size_t file_len;
	int rc;

	const struct cbor_attr_t dload_attr[] = {
		{
			.attribute = "off",
			.type = CborAttrUnsignedIntegerType,
			.addr.uinteger = &off,
		},
		{
			.attribute = "name",
			.type = CborAttrTextStringType,
			.addr.string = path,
			.len = sizeof path,
		},
		{ 0 },
	};

	off = ULLONG_MAX;
	rc = cbor_read_object(&ctxt->it, dload_attr);
	if (rc != 0 || off == ULLONG_MAX) {
		return MGMT_ERR_EINVAL;
	}

	/* Only the response to the first download request contains the total file
     * length.
     */
	if (off == 0) {
		rc = fs_mgmt_impl_filelen(path, &file_len);
		if (rc != 0) {
			return rc;
		}
	}

	/* Read the requested chunk from the file. */
	rc = fs_mgmt_impl_read(path, off, FS_MGMT_DL_CHUNK_SIZE, file_data,
			       &bytes_read);
	if (rc != 0) {
		return rc;
	}

	/* Encode the response. */
	err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "off");
	err |= cbor_encode_uint(&ctxt->encoder, off);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "data");
	err |= cbor_encode_byte_string(&ctxt->encoder, file_data, bytes_read);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "rc");
	err |= cbor_encode_int(&ctxt->encoder, MGMT_ERR_EOK);
	if (off == 0) {
		err |= cbor_encode_text_stringz(&ctxt->encoder, "len");
		err |= cbor_encode_uint(&ctxt->encoder, file_len);
	}

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}

/**
 * Encodes a file upload response.
 */
static int fs_mgmt_file_upload_rsp(struct mgmt_ctxt *ctxt, int rc,
				   unsigned long long off)
{
	CborError err;

	err = 0;
	err |= cbor_encode_text_stringz(&ctxt->encoder, "rc");
	err |= cbor_encode_int(&ctxt->encoder, rc);
	err |= cbor_encode_text_stringz(&ctxt->encoder, "off");
	err |= cbor_encode_uint(&ctxt->encoder, off);

	if (err != 0) {
		return MGMT_ERR_ENOMEM;
	}

	return 0;
}

/**
 * Command handler: fs file (write)
 */
static int fs_mgmt_file_upload(struct mgmt_ctxt *ctxt)
{
	uint8_t file_data[FS_MGMT_UL_CHUNK_SIZE];
	char file_name[FS_MGMT_PATH_SIZE + 1];
	unsigned long long len;
	unsigned long long off;
	size_t data_len;
	size_t new_off;
	int rc;

	const struct cbor_attr_t uload_attr[5] = {
		[0] = { .attribute = "off",
			.type = CborAttrUnsignedIntegerType,
			.addr.uinteger = &off,
			.nodefault = true },
		[1] = { .attribute = "data",
			.type = CborAttrByteStringType,
			.addr.bytestring.data = file_data,
			.addr.bytestring.len = &data_len,
			.len = sizeof(file_data) },
		[2] = { .attribute = "len",
			.type = CborAttrUnsignedIntegerType,
			.addr.uinteger = &len,
			.nodefault = true },
		[3] = { .attribute = "name",
			.type = CborAttrTextStringType,
			.addr.string = file_name,
			.len = sizeof(file_name) },
		[4] = { 0 },
	};

	len = ULLONG_MAX;
	off = ULLONG_MAX;
	rc = cbor_read_object(&ctxt->it, uload_attr);
	if (rc != 0 || off == ULLONG_MAX || file_name[0] == '\0') {
		return MGMT_ERR_EINVAL;
	}

	if (off == 0) {
		/* Total file length is a required field in the first chunk request. */
		if (len == ULLONG_MAX) {
			return MGMT_ERR_EINVAL;
		}

		fs_mgmt_ctxt.uploading = true;
		fs_mgmt_ctxt.off = 0;
		fs_mgmt_ctxt.len = len;
	} else {
		if (!fs_mgmt_ctxt.uploading) {
			return MGMT_ERR_EINVAL;
		}

		if (off != fs_mgmt_ctxt.off) {
			/* Invalid offset.  Drop the data and send the expected offset. */
			return fs_mgmt_file_upload_rsp(ctxt, MGMT_ERR_EINVAL,
						       fs_mgmt_ctxt.off);
		}
	}

	new_off = fs_mgmt_ctxt.off + data_len;
	if (new_off > fs_mgmt_ctxt.len) {
		/* Data exceeds image length. */
		return MGMT_ERR_EINVAL;
	}

	if (data_len > 0) {
		/* Write the data chunk to the file. */
		rc = fs_mgmt_impl_write(file_name, off, file_data, data_len);
		if (rc != 0) {
			return rc;
		}
		fs_mgmt_ctxt.off = new_off;
	}

	if (fs_mgmt_ctxt.off == fs_mgmt_ctxt.len) {
		/* Upload complete. */
		fs_mgmt_ctxt.uploading = false;
	}

	/* Send the response. */
	return fs_mgmt_file_upload_rsp(ctxt, 0, fs_mgmt_ctxt.off);
}

void bt610_mgmt_register_group(void)
{
	mgmt_register_group(&fs_mgmt_group);
}
