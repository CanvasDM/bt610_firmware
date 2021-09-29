/**
 * @file Version.h
 * @brief
 *
 * Copyright (c) 2020-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef __VERSION_H__
#define __VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/* Application Firmware Version                                               */
/******************************************************************************/
#define VERSION_MAJOR 1
#define VERSION_MINOR 27
#define VERSION_PATCH 5
#define VERSION_STRING                                                         \
	STRINGIFY(VERSION_MAJOR)                                               \
	"." STRINGIFY(VERSION_MINOR) "." STRINGIFY(VERSION_PATCH)

#ifdef __cplusplus
}
#endif

#endif /* __VERSION_H__ */
