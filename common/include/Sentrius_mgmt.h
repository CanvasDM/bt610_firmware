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

#ifndef H_SENTRIUS_MGMT_
#define H_SENTRIUS_MGMT_

#ifdef __cplusplus
extern "C" {
#endif

#include "mgmt/mgmt.h"

#define MGMT_GROUP_ID_SENTRIUS   65

// pystart - mgmt handler function defines
mgmt_handler_fn Sentrius_mgmt_GetParameter;
mgmt_handler_fn Sentrius_mgmt_SetParameter;
mgmt_handler_fn Sentrius_mgmt_Echo;
// pyend
/**
 * Command IDs for file system management group.
 */
// pystart - mgmt function indices
#define SENTRIUS_MGMT_ID_GETPARAMETER                          1
#define SENTRIUS_MGMT_ID_SETPARAMETER                          2
#define SENTRIUS_MGMT_ID_ECHO                                  3
// pyend

/**
 * @brief Registers the file system management command handler group.
 */ 
void Sentrius_mgmt_register_group(void);

#ifdef __cplusplus
}
#endif

#endif
