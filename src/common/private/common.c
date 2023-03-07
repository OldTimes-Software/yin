// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_filesystem.h>
#include <plcore/pl_console.h>

#include <yin/node.h>

#include "common.h"

int logLevelPrint;
int logLevelWarn;

void Common_Pkg_RegisterInterface( void );

void Common_Initialize( void )
{
	logLevelPrint = PlAddLogLevel( "common", PL_COLOUR_WHITE, true );
	logLevelWarn  = PlAddLogLevel( "common/warning", PL_COLOUR_YELLOW, true );

	Message( "Common Library initialized\n" );

	YnNode_SetupLogs();

	Common_Pkg_RegisterInterface();
}

static PLPath appDataPath = "";

const char *Common_GetAppDataDirectory( void )
{
	if ( *appDataPath != '\0' )
	{
		return appDataPath;
	}

	if ( PlGetApplicationDataDirectory( "yin", appDataPath, sizeof( appDataPath ) ) != NULL )
	{
		return appDataPath;
	}

	Warning( "Failed to fetch application data directory: %s\n", PlGetError() );

	snprintf( appDataPath, sizeof( appDataPath ), "." );
	return appDataPath;
}

YNNodeBranch *Common_GetConfig( const char *name )
{
	// first attempt to load from local dir
	PLPath configPath;
	snprintf( configPath, sizeof( configPath ), "%s/%s.cfg.n", Common_GetAppDataDirectory(), name );
	YNNodeBranch *root = YnNode_LoadFile( configPath, "config" );
	if ( root != NULL )
	{
		return root;
	}

	// otherwise attempt to load from app data dir instead
	snprintf( configPath, sizeof( configPath ), "%s.cfg.n", name );
	root = YnNode_LoadFile( configPath, "config" );
	if ( root == NULL )
	{
		Warning( "Failed to load user config file: %s\n"
		         "Creating empty config.\n",
		         YnNode_GetErrorMessage() );
		root = YnNode_PushBackObject( NULL, "config" );
	}

	return root;
}

bool Common_WriteConfig( YNNodeBranch *root, const char *name )
{
	PLPath configPath;
	snprintf( configPath, sizeof( configPath ), "%s/%s.cfg.n", Common_GetAppDataDirectory(), name );
	YnNode_WriteFile( configPath, root, YN_NODE_FILE_UTF8 );
	return true;
}
