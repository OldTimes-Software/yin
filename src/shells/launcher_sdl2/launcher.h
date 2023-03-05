/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_console.h>

#include <plgraphics/plg.h>
#include <plgraphics/plg_driver_interface.h>

#include "engine_public_base.h"

extern int launcherLog;

#define Print( ... ) PlLogMessage( launcherLog, __VA_ARGS__ )
#define PrintWarn( ... )                                         \
	{                                                            \
		PlLogMessage( launcherLog, __VA_ARGS__ );                \
		OS_Shell_DisplayMessageBox( OS_MESSAGE_WARNING, __VA_ARGS__ ); \
	}

#ifdef NDEBUG
#	define PrintError( ... )                                      \
		{                                                          \
			PlLogMessage( launcherLog, __VA_ARGS__ );              \
			OS_Shell_DisplayMessageBox( OS_MESSAGE_ERROR, __VA_ARGS__ ); \
			exit( EXIT_FAILURE );                                  \
		}
#else
#	define PrintError( ... )                                      \
		{                                                          \
			PlLogMessage( launcherLog, __VA_ARGS__ );              \
			OS_Shell_DisplayMessageBox( OS_MESSAGE_ERROR, __VA_ARGS__ ); \
			abort();                                               \
		}
#endif
