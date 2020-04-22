/* main.c - Application main entry point */

/*
 * Copyright (c) 2019 Laird Connectivity
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Framework.h"
#include "ControlTask.h"

void main(void)
{
	Framework_Initialize();
	
	ControlTask_Initialize();
	ControlTask_Thread();
}
