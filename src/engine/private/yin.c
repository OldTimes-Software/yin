// Copyright © 2020-2023 OldTimes Software, Mark E Sowden <hogsy@oldtimes-software.com>

#include <plmodel/plm.h>
#include <plgraphics/plg_driver_interface.h>

#include "engine_private.h"
#include "engine_public_game.h"
#include "filesystem.h"
#include "model.h"

#include "client/client.h"
#include "client/client_input.h"
#include "editor/editor.h"

#include "server/server.h"
#include "net/net.h"
#include "node/public/node.h"

/****************************************
 * PRIVATE
 ****************************************/

static unsigned int numTicks = 0;

static NLNode *engineConfig;
static NLNode *userConfig;

static bool engineTerminalMode = false;
static bool engineInitialized  = false;

/****************************************
 * PUBLIC
 ****************************************/

const int ENGINE_VERSION[ 3 ] = { VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH };

NLNode *Engine_GetConfig( void ) { return engineConfig; }
NLNode *Engine_GetUserConfig( void ) { return userConfig; }

bool Engine_IsTerminalMode( void ) { return engineTerminalMode; }

bool Engine_Initialize( const char *config )
{
	// Call this first, so we can buffer console output
	Console_Initialize();

	PRINT( "Yin %d (%s / (%s:%s, %s)), Copyright (C) 2020-2023 Mark E Sowden\n",
	       VERSION_MAJOR,
	       ENGINE_VERSION_STR,
	       GIT_BRANCH, GIT_COMMIT_COUNT, GIT_COMMIT_HASH );
	PRINT( "Current working directory: \"%s\"\n", PlGetWorkingDirectory() );

	engineTerminalMode = PlHasCommandLineArgument( "cmd" );
	if ( engineTerminalMode )
	{
		PRINT( "Operating in command-line mode!\n" );
	}

	Console_RegisterVariables( engineTerminalMode );
	Console_RegisterCommands( engineTerminalMode );

	// Need to do this before anything else IO related
	FileSystem_MountBaseLocations();

	// And now we can fetch the engine config that provides mount locations, aliases and more
	if ( config == NULL )
	{
		PRINT( "Shell didn't provide config - "
		       "checking for command-line argument, otherwise will use default.\n" );
		config = ENGINE_BASE_CONFIG;
	}
	const char *configPath = PlGetCommandLineArgumentValue( "-config" );
	engineConfig           = NL_LoadFile( configPath != NULL ? configPath : config, "config" );
	if ( engineConfig == NULL )
	{
		PRINT_WARNING( "Failed to open engine config: %s\n", NL_GetErrorMessage() );
		return false;
	}

	userConfig = NL_LoadFile( FileSystem_GetUserConfigLocation(), "config" );
	if ( userConfig == NULL )
	{
		PRINT( "No existing user config found, will use defaults.\n" );
		userConfig = NL_PushBackObj( NULL, "config" );
	}

	FileSystem_SetupConfig( engineConfig );
	FileSystem_MountLocations();

	PRINT( "Initializing core services...\n" );

	// TODO: move these somewhere more appropriate??
	PlmRegisterModelLoader( "mdl.n", Model_Cache );

	Profiler_Initialize();
	Scheduler_Initialize();
	MemoryManager_Initialize();
	Net_Initialize();

	Server_Initialize();
	Client_Initialize();

	Game_Initialize();

	PRINT( "Initialization complete!\n" );

	engineInitialized = true;

	return true;
}

void Engine_Shutdown( void )
{
	PRINT( "Shutting down...\n" );

	Sch_FlushTasks();

	Game_Shutdown();
	Editor_Shutdown();

	Client_Shutdown();
	Server_Shutdown();
	Console_Shutdown();
	MemoryManager_Shutdown();
	Scheduler_Shutdown();
	Net_Shutdown();
	FileSystem_ClearMountedLocations();

	OS_Shell_Shutdown();

	engineInitialized = false;
}

unsigned int Engine_GetNumTicks( void )
{
	return numTicks;
}

void Engine_TickFrame( void )
{
	if ( !engineInitialized )
		return;

	PROFILE_START( PROFILE_SIM_ALL );

	Sch_RunTasks();

	PROFILE_START( PROFILE_TICK_CLIENT );
	Client_Tick();
	PROFILE_END( PROFILE_TICK_CLIENT );

	PROFILE_START( PROFILE_TICK_SERVER );
	Server_Tick();
	PROFILE_END( PROFILE_TICK_SERVER );

	numTicks++;

	PROFILE_END( PROFILE_SIM_ALL );

	Profiler_UpdateGraphs();
	Profiler_EndFrame();
}

bool YinCore_IsEngineRunning( void )
{
	/* always running */
	return engineInitialized;
}

void YinCore_RenderFrame( YRViewport *viewport )
{
	if ( !engineInitialized )
		return;

	assert( viewport != NULL );
	if ( viewport == NULL )
	{
		PRINT_WARNING( "Attempted to draw without a valid viewport!\n" );
		return;
	}

	PROFILE_START( PROFILE_DRAW_ALL );
	Client_Draw( viewport );
	PROFILE_END( PROFILE_DRAW_ALL );

	Profiler_UpdateGraphs();
	Profiler_EndFrame();
}

void Engine_HandleKeyboardEvent( int key, unsigned int keyState )
{
	Client_Input_HandleKeyboardEvent( key, keyState );
}

bool Client_Console_HandleTextEvent( const char *key );

void Engine_HandleTextEvent( const char *key )
{
	if ( Client_Console_HandleTextEvent( key ) )
		return;
}

void Engine_HandleMouseButtonEvent( int button, OSInputState buttonState )
{
	Client_Input_HandleMouseButtonEvent( button, buttonState );
}

void Engine_HandleMouseWheelEvent( float x, float y )
{
	Client_Input_HandleMouseWheelEvent( x, y );
}

void YinCore_HandleMouseMotionEvent( int x, int y )
{
	Client_Input_HandleMouseMotionEvent( x, y );
}
