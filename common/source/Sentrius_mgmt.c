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
#include "Sentrius_mgmt.h"
#include "Sentrius_mgmt_impl.h"
#include "Sentrius_mgmt_config.h"

static mgmt_handler_fn fs_mgmt_file_download;
static mgmt_handler_fn fs_mgmt_file_upload;

static const struct mgmt_handler sentrius_mgmt_handlers[] = {
	// pystart - mgmt handlers
     [1] = {
        .mh_read = fs_mgmt_file_download,
        .mh_write = fs_mgmt_file_upload,
     },
	// pyend
};

static int fs_mgmt_file_download(struct mgmt_ctxt *ctxt)
{
     uint8_t off = 0;
     uint8_t file_len = 12;
     CborError err;
     /* Encode the response. */
     err = 0;
     err |= cbor_encode_text_stringz(&ctxt->encoder, "off");
     err |= cbor_encode_uint(&ctxt->encoder, off);
     err |= cbor_encode_text_stringz(&ctxt->encoder, "data");
     err |= cbor_encode_text_stringz(&ctxt->encoder, "rc");
     if (off == 0) {
          err |= cbor_encode_text_stringz(&ctxt->encoder, "len");
          err |= cbor_encode_uint(&ctxt->encoder, file_len);
     }
     return 0;
}

static int fs_mgmt_file_upload(struct mgmt_ctxt *ctxt)
{
     return 1;
}


#define SENTRIUS_MGMT_HANDLER_CNT                                              \
	(sizeof sentrius_mgmt_handlers / sizeof sentrius_mgmt_handlers[0])

static struct mgmt_group sentrius_mgmt_group = {
	.mg_handlers = sentrius_mgmt_handlers,
	.mg_handlers_count = SENTRIUS_MGMT_HANDLER_CNT,
	.mg_group_id = MGMT_GROUP_ID_SENTRIUS,
};

void Sentrius_mgmt_register_group(void)
{
	mgmt_register_group(&sentrius_mgmt_group);
}
