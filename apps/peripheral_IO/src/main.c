/* main.c - Application main entry point */

/*
 * Copyright (c) 2019 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Framework.h"
#include "ControlTask.h"
#include "BspSupport.h"

void main(void)
{
	BSP_Init();
	Framework_Initialize();

	ControlTask_Initialize();
	ControlTask_Thread();
}
