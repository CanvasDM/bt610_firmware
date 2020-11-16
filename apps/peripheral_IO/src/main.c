/* main.c - Application main entry point */

/*
 * Copyright (c) 2019 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Framework.h"
#include "ControlTask.h"
<<<<<<< HEAD
#include "BspSupport.h"

void main(void)
{
	BSP_Init();
	Framework_Initialize();

=======

void main(void)
{
	Framework_Initialize();
	
>>>>>>> made changes to the firmware filese that were updated
	ControlTask_Initialize();
	ControlTask_Thread();
}
