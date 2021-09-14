/* main.c - Application main entry point */

/*
 * Copyright (c) 2019-2021 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "ControlTask.h"
#include "BspSupport.h"

void main(void)
{
	BSP_Init();
	ControlTask_Initialize();
	ControlTask_Thread();

}
