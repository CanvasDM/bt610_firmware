/******************************************************************************/
//! @file ControlTask.h
//!
//! @brief 
//!
//! @copyright Copyright 2019 Laird
//!            All Rights Reserved.
/******************************************************************************/

#ifndef CONTROL_TASK_H
#define CONTROL_TASK_H

//EXTERN_C_BEGIN

/******************************************************************************/
/* Global Constants, Macros and Type Definitions                              */
/******************************************************************************/

//! The main thread can either have a stack large enough to do initialization, or
//! the control thread can be the "main" thread, or
//!
#ifndef CONTROL_TASK_USES_MAIN_THREAD
#define CONTROL_TASK_USES_MAIN_THREAD 1
#endif

/******************************************************************************/
/* Global Data Definitions                                                    */
/******************************************************************************/
// NA

/******************************************************************************/
/* Global Function Prototypes                                                 */
/******************************************************************************/

//-----------------------------------------------
//!
void ControlTask_Initialize(void);

//-----------------------------------------------
//! If CONTROL_TASK_USES_MAIN_THREAD is non-zero then this needs to part of main.
//!
void ControlTask_Thread(void);

//EXTERN_C_END

#endif

// end
