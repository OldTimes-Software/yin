/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

/****************************************
 * SCHEDULER
 ****************************************/

typedef void ( *SchedulerCallback )( void *userData, double delta );

void YnCore_InitializeScheduler( void );
void YnCore_ShutdownScheduler( void );
unsigned int Sch_GetNumTasks( void );
const char  *Sch_GetTaskDescription( unsigned int index, double *delay );
bool         Sch_IsTaskRunning( const char *desc );
void         Sch_PushTask( const char *desc, SchedulerCallback callback, void *userData, double delay );
void YinCore_TickTasks( void );
void YnCore_FlushTasks( void );
void         Sch_PrintPendingTasks( void );
void         Sch_KillTask( const char *desc );
void         Sch_SetTaskDelay( const char *desc, double delay );
